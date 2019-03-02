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
#include "spark_wiring_wifi.h"

/* Includes ---- standard libraries ------------------------------------------*/
#include "stdarg.h"
#include "math.h"

/* Includes ---- application -------------------------------------------------*/
#include "brewferm.h"
#include "WiFi_Info.h"
#include "Sensors.h"
#include "relays.h"
#include "comms.h"
#include "storage.h"

/* Includes ---- 3rd Party ---------------------------------------------------*/
#include "libs/pid.h"

PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(3);

// SSID and password #define'd in WiFi_Info.h
const char* SSID = MY_SSID;
const char* PASSWORD = MY_PASSWORD;

#if Wiring_WiFi
STARTUP(System.enable(SYSTEM_FLAG_WIFITESTER_OVER_SERIAL1));
#endif

SENSORS mySensors;

double fake_beer_temp;
double fake_chamber_temp;

mode_heat_cool heat_cool_status;

time_t ts_lastSim;

double cool_limit, heat_limit;

double beer_temp_target, beer_temp_actual, beer_pid_out;
double chamber_target_temp, chamber_temp_actual, chamber_pid_out;
double control_signal;

PID beerTempPID(&beer_temp_actual, &beer_pid_out, &beer_temp_target,
  BEER_P, BEER_I, BEER_D,
  PID::DIRECT);

PID chamberTempPID(&chamber_temp_actual, &chamber_pid_out, &chamber_target_temp,
    CHAMBER_P, CHAMBER_I, CHAMBER_D,
    PID::DIRECT);

RELAYS myRelays;
COMMS myComms;
STORAGE myStorage;

//-----------------------------------------------------------------------------
/* Set the upper and lower bounds for the beer PID output value --------------*/
/*   which in our case will be our target fermentation chamber temperature ---*/
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

//-----------------------------------------------------------------------------
/* Define LED modes ----------------------------------------------------------*/
void initIndicators(void) {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
}

//-----------------------------------------------------------------------------
/* Initialize and prime beer temp PID ----------------------------------------*/
void initBeerPID(void) {
  // Init pid fields
  beer_temp_target = DEFAULT_SP;
  beer_temp_target = myStorage.retrieve_beer_temp_target();
  beer_temp_actual = beer_temp_target;

  // Turn on pid
  beerTempPID.SetMode(PID::AUTOMATIC);
  beerTempPID.SetSampleTime(5*1000);  // Five seconds is fast enough to react

  chamberTempPID.SetMode(PID::AUTOMATIC);
  chamberTempPID.SetSampleTime(5*1000);

  control_signal = 50;
  chamber_temp_actual = 60;

  adjustChamberTempLimits(beer_temp_target);
}

//-----------------------------------------------------------------------------
/* Do initial network setup --------------------------------------------------*/
void initNetworking(void) {
  System.disableUpdates();
  WiFi.on();

  delay(4 * DELAY_TIME);
  WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
  delay(4 * DELAY_TIME);
  if (WiFi.ready()) {
      Particle.connect();
      delay(DELAY_TIME);
      Particle.syncTime();
      delay(DELAY_TIME);
      // Particle.disconnect();
      delay(DELAY_TIME);
  }
}

//-----------------------------------------------------------------------------
/* Set the indicators for beer in temp rage ----------------------------------*/
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

//-----------------------------------------------------------------------------
/* ---------------------------------------------------------------------------*/
void get_current_temperatures() {
  mySensors.refresh();

  // Apply low pass filters to the temperature sensors to eliminate
  // noise, the temps change slowly so we're Okay with a slow change
  // rate on the temp readings. Chamber changes quicker than beer, so
  // it gets a high alpha parameter.

  chamber_temp_actual += 0.7 * (mySensors.GetTempF(SENSORS::CHAMBER) -
                                  chamber_temp_actual);

  beer_temp_actual += 0.2 * (mySensors.GetTempF(SENSORS::BEER) -
                               beer_temp_actual);

  delay(DELAY_TIME);
}

/* ---------------------------------------------------------------------------*/
void blinkStayAlive(void) {
  digitalWrite(LED_PIN, HIGH);
  delay(2 * DELAY_TIME);

  digitalWrite(LED_PIN, LOW);
  delay(DELAY_TIME);
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

  beerTempPID.Compute();

  // Apply low pass filter to PID output to reduce on/off signals to cooling
  // the alpha parameter of y += alpha * (x-y); needs to stay close to
  // unity (1.0) to keep the lag time down

  chamber_target_temp += 0.6 * (beer_pid_out - chamber_target_temp);
  delay(DELAY_TIME);

  chamberTempPID.Compute();
  control_signal += 0.6 * (chamber_pid_out - control_signal);

  delay(DELAY_TIME);
}

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

  if (control_signal < cool_limit) {
    myRelays.coolON();
    heat_cool_status = COOLING_ON;
  } else if (control_signal > heat_limit) {
    myRelays.heatON();
    heat_cool_status = HEATER_ON;
  } else {
    myRelays.idle();
    heat_cool_status = IDLE;
  }

  delay(DELAY_TIME);
}  // RELAYS::control_HeatCool

/* ---------------------------------------------------------------------------*/
void communicate() {
  myComms.sendStatus(beer_temp_target, beer_temp_actual, chamber_target_temp,
    chamber_temp_actual, control_signal, myRelays.getHeatCoolStatus(),
    heat_cool_status);

  myComms.processIncomingMessages();

  if (myComms.setPointAvailable()) {
    beer_temp_target = myComms.getSetPoint();
    adjustChamberTempLimits(beer_temp_target);
    myStorage.store_beer_temp_target(beer_temp_target);
  }

  setIndicatorLEDs(beer_temp_actual, beer_temp_target);
  delay(DELAY_TIME);  // temps don't change too quickly, we can wait

  blinkStayAlive();
}

/* This function is called once by the OS at device start up -----------------*/
void setup() {
    initIndicators();
    initBeerPID();
    initNetworking();

    myComms.init();

    // prime the pump for adjustbeer_temp_actual()
    mySensors.refresh();
    // last_adjust = mySensors.GetTempF(SENSORS::BEER);

    Serial.begin(SERIAL_BAUD);

    // these are fake values for test when no sensors attached
    fake_beer_temp = 64.7;
    fake_chamber_temp = 70.0;

    cool_limit = COOL_LIMIT2;
    heat_limit = HEAT_LIMIT2;

    ts_lastSim = Time.now();
}

/* This function loops forever -----------------------------------------------*/
void loop() {
    // This will run in a loop
    get_current_temperatures();  // reads the sensosrs to get temps
    run_calculations();
    control_HeatCool();
    communicate();
}
