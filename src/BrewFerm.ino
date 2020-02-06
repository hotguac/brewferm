/*
 ******************************************************************************
  Copyright (c) 2019 Joe Kokosa  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

/* Includes ---- photon vendor defined ---------------------------------------*/
#include "application.h"

/* Includes ---- standard libraries ------------------------------------------*/
#include "stdarg.h"
#include "math.h"

/* Includes ---- application -------------------------------------------------*/
#include "brewferm.h"
#include "sensors.h"
#include "relays.h"
#include "storage.h"
#include "jpid.h"

/* Includes ---- 3rd Party ---------------------------------------------------*/
//#include <pid.h>

PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(4);
SYSTEM_MODE(AUTOMATIC);

int OTA_enabled;

SENSORS mySensors;

double fake_beer_temp;
double fake_chamber_temp;

mode_heat_cool heat_cool_status;

String target_action("WAIT");
String current_action("WAIT");

String system_status;

time_t ts_lastSim;
time_t ts_last_loop;
time_t ts_next_loop;
time_t ts_now;
time_t ts_last_publish;

double cool_limit, heat_limit;

boolean paused = false;

double beer_temp_target, beer_temp_actual, beer_pid_out;
double chamber_target_temp, chamber_temp_actual, chamber_pid_out;
double percent_heat = 0.0, percent_cool = 0.0;
double control_signal;

double beerITerm, chamberITerm;

double uptime = 0.0;
double cooltime = 0.0;
double heattime = 0.0;



PID beerTempPID(&beer_temp_actual, &beer_pid_out, &beer_temp_target,
  BEER_P, BEER_I, BEER_D,
  PID::DIRECT);

PID chamberTempPID(&chamber_temp_actual, &chamber_pid_out, &chamber_target_temp,
    CHAMBER_P, CHAMBER_I, CHAMBER_D,
    PID::DIRECT);

RELAYS myRelays;
STORAGE myStorage;

/* ---------------------------------------------------------------------------*/
// Set the upper and lower bounds for the beer PID output value
//   which in our case will be our target fermentation chamber temperature
/* ---------------------------------------------------------------------------*/
void adjustChamberTempLimits(double sp) {
  if (sp > FERM_CRASH_SPLIT) {
    // We're still in fermentation mode
    beerTempPID.SetOutputLimits(sp - LIMIT_RANGE_LOW, sp + LIMIT_RANGE_HIGH);
  } else {
    // We're cold crashing now
    beerTempPID.SetOutputLimits(MIN_OUTPUT_TEMP, sp + LIMIT_RANGE_HIGH);
  }

  chamberTempPID.SetOutputLimits(1, 99);
}

// ----------------
// Define LED modes
// ----------------
void initIndicators(void) {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
}

// ----------------------------------
// Initialize and prime beer temp PID
// ----------------------------------
void initBeerPID(void) {
  // Init pid fields
  beer_temp_target = DEFAULT_SP;
  beer_temp_target = myStorage.retrieve_beer_temp_target();
  chamber_target_temp = beer_temp_target;

  mySensors.refresh();
  beer_temp_actual = mySensors.GetTempF(SENSORS::BEER);
  chamber_temp_actual = mySensors.GetTempF(SENSORS::CHAMBER);

    // Turn on pid
  beerTempPID.SetMode(PID::AUTOMATIC);
  beerTempPID.SetSampleTime(BEER_SAMPLETIME*1000);

  chamberTempPID.SetMode(PID::AUTOMATIC);
  chamberTempPID.SetSampleTime(CHAMBER_SAMPLETIME*1000);

  control_signal = 50;

  adjustChamberTempLimits(beer_temp_target);

  //run_calculations();

  beerTempPID.Initialize(beer_temp_actual);
  chamberTempPID.Initialize(50.0);

  delay(30*1000);
  run_calculations();
  update_system_status();
}

/* ---------------------------------------------------------------------------*/
// Do initial network setup
/* ---------------------------------------------------------------------------*/
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

void checkNetworking(void) {
  if (!Particle.connected()) {
    Particle.connect();
  }

  if (Particle.timeSyncedLast() > ONE_DAY_MILLIS) {
    Particle.syncTime();
  }

  OTA_enabled = System.updatesEnabled();

  if (!OTA_enabled) {
    System.enableUpdates();
  }
}

/* ---------------------------------------------------------------------------*/
// Set the indicators for beer in temp rage
/* ---------------------------------------------------------------------------*/
void setIndicatorLEDs(double beer_temp_actual, double beer_temp_target) {
  double diff = fabs(beer_temp_actual - beer_temp_target);

  if (diff < INDICATE_OK) {
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
  } else if (diff < INDICATE_CLOSE) {
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, HIGH);
  } else {
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
  }
}

/* ---------------------------------------------------------------------------*/
// Read sensors to refresh actual temperature values
/* ---------------------------------------------------------------------------*/
void get_current_temperatures() {
  mySensors.refresh();

  // Apply low pass filters to the temperature sensors to eliminate
  // noise, the temps change slowly so we're Okay with a slow change
  // rate on the temp readings. Chamber changes quicker than beer, so
  // it gets a high alpha parameter.

  chamber_temp_actual += 0.99 * (mySensors.GetTempF(SENSORS::CHAMBER) -
                                  chamber_temp_actual);

  beer_temp_actual += 0.99 * (mySensors.GetTempF(SENSORS::BEER) -
                               beer_temp_actual);
}

/* ---------------------------------------------------------------------------*/
// Flash the indicator on the Photon so we now the loop is still running
/* ---------------------------------------------------------------------------*/
void blinkAndSetPace(void) {
  digitalWrite(LED_PIN, HIGH);
  delay(500); // flash for half second; off time determined by delay below
  digitalWrite(LED_PIN, LOW);

  ts_next_loop = ts_last_loop + MIN_LOOP_TIME;
  ts_now = Time.now();

  if (ts_now < ts_next_loop) {
    delay((ts_next_loop - ts_now) * 1000);
  }

  ts_last_loop = Time.now();
}

/* ---------------------------------------------------------------------------*/
void run_calculations() {
  // this is just for testing without any sensors attached
  // we'll fake the beer temp moving towards the chamber temp and
  // the chamber temp moving towards chamber target temp

#ifdef RUN_SIMULATION
  /* Actual value's from my plant

  Observed heating rate when ambient temp is low 30's F was 0.1F/minute
  Observed cooling rate when ambient temp is low 30's F was 0.6F/minute

  */
#define SIM_HEAT_RISE_PER_SECOND (0.09/60)
#define SIM_COOL_DROP_PER_SECOND (0.6/60)
#define SIM_TEMP_XFER_BEER_PER_SECOND (0.00045/60)
#define SIM_TEMP_XFER_CHAMBR_PER_SECOND (0.00225/60)

  double lastSim_seconds = (Time.now() - ts_lastSim);
  ts_lastSim = Time.now();

  // beer temp react's to chamber temp
  fake_beer_temp += (fake_chamber_temp - fake_beer_temp) *
                    (SIM_TEMP_XFER_BEER_PER_SECOND * lastSim_seconds);

  if (myRelays.getHeatCoolStatus() == HEATER_ON) {
      fake_chamber_temp += SIM_HEAT_RISE_PER_SECOND * lastSim_seconds;
  } else if (myRelays.getHeatCoolStatus() == COOLING_ON) {
      fake_chamber_temp -= SIM_COOL_DROP_PER_SECOND *lastSim_seconds;
  } else {  // we're coasting in idle so beer slowly affects chamber
      fake_chamber_temp += (fake_beer_temp - fake_chamber_temp) *
                          (SIM_TEMP_XFER_CHAMBR_PER_SECOND * lastSim_seconds);
  }

  ts_lastSim = Time.now();

  chamber_temp_actual = fake_chamber_temp;
  beer_temp_actual = fake_beer_temp;
  // end of fake temp code
  //
#endif

  // Apply low pass filter to PID output to reduce on/off signals to cooling
  // the alpha parameter of y += alpha * (x-y); needs to stay close to
  // unity (1.0) to keep the lag time down
  beerTempPID.Compute();
  chamber_target_temp += 0.99 * (beer_pid_out - chamber_target_temp);

  chamberTempPID.Compute();
  control_signal += 0.99 * (chamber_pid_out - control_signal);
}

/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void control_HeatCool(void) {
  /*
  #define COOL_LIMIT1  10
  #define COOL_LIMIT2  30
  #define HEAT_LIMIT2  60
  #define HEAT_LIMIT1  90
  */

  if (cool_limit == COOL_LIMIT1) {
    if (control_signal > COOL_LIMIT2) {
      cool_limit = COOL_LIMIT2;
    }
  }

  if (cool_limit == COOL_LIMIT2) {
    if (control_signal < COOL_LIMIT1) {
      cool_limit = COOL_LIMIT1;
    }
  }

  if (heat_limit == HEAT_LIMIT2) {
    if (control_signal > HEAT_LIMIT1) {
      heat_limit = HEAT_LIMIT1;
    }
  }

  if (heat_limit == HEAT_LIMIT1) {
    if (control_signal < HEAT_LIMIT2) {
      heat_limit = HEAT_LIMIT2;
    }
  }

  if (paused) {
    myRelays.idle();
    heat_cool_status = IDLE;
    target_action = "Paused";
  } else if (control_signal < cool_limit) {
    myRelays.coolON();
    heat_cool_status = COOLING_ON;
    target_action = "Cooling";
  } else if (control_signal > heat_limit) {
    myRelays.heatON();
    heat_cool_status = HEATER_ON;
    target_action = "Heating";
  } else {
    myRelays.idle();
    heat_cool_status = IDLE;
    target_action = "Idle";
  }

  switch (myRelays.getHeatCoolStatus()) {
    case COOLING_ON:
      current_action = "Cooling";
      break;
    case HEATER_ON:
      current_action = "Heating";
      break;
    case IDLE:
      if (paused) {
        current_action = "Paused";
      } else {
        current_action = "Idle";
      }
      break;
  }

}

/* ---------------------------------------------------------------------------*/
// Cloud function to set target beer temp
/* ---------------------------------------------------------------------------*/
int setBeerTarget(String temp_target) {
    int result = 0;
    float temp = atof(temp_target); // returns 0.0 if atof fails

    if (temp > 0.0) {
      beer_temp_target = temp;
      beerTempPID.SynchITerm();
      adjustChamberTempLimits(beer_temp_target);

      myStorage.store_beer_temp_target(beer_temp_target);
    } else {
      result = -1;
    }

    return result;
}

/* ---------------------------------------------------------------------------*/
// Cloud function to pause heat/cool functions when not in use
/* ---------------------------------------------------------------------------*/
int setPause(String pause) {
    int result = 0;

    if (!pause.compareTo("YES")) {
      paused = true;
      myStorage.store_pause_state(true);
    } else if (!pause.compareTo("NO")) {
      paused = false;
      myStorage.store_pause_state(false);
    } else {
      result = -1;
    }

    return result;
}

/* ---------------------------------------------------------------------------*/
// Build status string for reporting to the cloud
/* ---------------------------------------------------------------------------*/
#define MAX_DATA_SIZE 622
void update_system_status() {

    // get times in seconds to calc work
    uptime = System.uptime();
    cooltime = myRelays.get_cool_runtime();
    heattime = myRelays.get_heat_runtime();

    percent_cool = (cooltime / uptime) * 100.0;
    percent_heat = (heattime / uptime) * 100.0;

    char buffer[MAX_DATA_SIZE];
    memset(buffer, 0, sizeof(buffer));

    snprintf(buffer, sizeof(buffer),
      "|BT:%.1f|BA:%.1f|CT:%.1f|CA:%.1f|CS:%.1f|PH:%.1f|PC:%.1f|BI:%.1f|CI:%.1f|end",
        beer_temp_target,
        beer_temp_actual,
        chamber_target_temp,
        chamber_temp_actual,
        chamber_pid_out,
        percent_heat,
        percent_cool,
        beerTempPID.GetITerm(),
        chamberTempPID.GetITerm()
        );

    system_status.remove(0);
    system_status.concat("TA:");
    system_status.concat(target_action);
    system_status.concat("|CA:");
    system_status.concat(current_action);
    system_status.concat(buffer);

    if ((Time.now() - ts_last_publish) > SEND_STATUS_INTERVAL) {
      if (Particle.connected()) {
        Particle.publish("SystemStatus", system_status, PRIVATE);
        ts_last_publish = Time.now();
      }
    }
}

/* ---------------------------------------------------------------------------*/
// This function is called once by the OS at device start up
/* ---------------------------------------------------------------------------*/
void setup() {
    Particle.variable("SystemStatus", system_status);
    Particle.variable("Uptime", uptime);
    Particle.variable("HeatTime", heattime);
    Particle.variable("CoolTime", cooltime);
    Particle.variable("BeerITerm", beerITerm);
    Particle.variable("ChamberITerm", chamberITerm);

    Particle.function("setBeerTarget", setBeerTarget);
    Particle.function("setPauseState", setPause);

    checkNetworking();
    initIndicators();
    initBeerPID();

    paused = myStorage.retrieve_pause_state();

    // prime the pump for adjustbeer_temp_actual()
    mySensors.refresh();

    // these are fake values for test when no sensors attached
    fake_beer_temp = 64.7;
    fake_chamber_temp = 70.0;

    cool_limit = COOL_LIMIT2;
    heat_limit = HEAT_LIMIT2;

    ts_lastSim = Time.now();
    ts_last_loop = ts_lastSim - 200; // pretend we last looped 200 seconds ago
    ts_last_publish = ts_last_loop;
}

/* ---------------------------------------------------------------------------*/
/* This function loops forever (firmware process loop)
/* ---------------------------------------------------------------------------*/
void loop() {
    checkNetworking();
    Particle.process();

    get_current_temperatures();  // reads the sensosrs to get temps
    Particle.process();

    if (!paused) {
      run_calculations();
      Particle.process();
      beerITerm = beerTempPID.GetITerm();
      chamberITerm = chamberTempPID.GetITerm();
    }

    control_HeatCool();
    Particle.process();

    update_system_status();
    Particle.process();

    setIndicatorLEDs(beer_temp_actual, beer_temp_target);
    blinkAndSetPace();
}
