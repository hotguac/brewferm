//*******************************************************************************
//  Copyright (c) 2019 Joe Kokosa  All rights reserved.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation, either
//  version 3 of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program; if not, see <http://www.gnu.org/licenses/>.
//******************************************************************************

/* Includes ---- photon vendor defined ---------------------------------------*/
#include "application.h"
#include "Particle.h"

/* Includes ---- standard libraries ------------------------------------------*/
#include "stdarg.h"
#include "math.h"

/* Includes ---- application -------------------------------------------------*/
#include "brewferm.h"
#include "sensors.h"
#include "relays.h"
#include "storage.h"
#include "indicators.h"
#include "jpid.h"

#define RUN_SIMULATION

int OTA_enabled;

SENSORS mySensors;

double fake_beer_temp;
double fake_chamber_temp;

mode_heat_cool heat_cool_status;

String target_action("WAIT");
String current_action("WAIT");

String system_status;
String system_tuning;

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

double beerP;
double beerI;
double beerD;

double chamberP;
double chamberI;
double chamberD;

int ble_max_count = 0;
String beaconMajor;

double tilt_sg = 0.0;

#if Wiring_BLE
const size_t SCAN_RESULT_MAX = 30;
BleCharacteristic tilt;
BleScanResult scanResults[SCAN_RESULT_MAX];
BlePeerDevice peer;
int ble_device_count = 0;
#endif

PID beerTempPID(&beer_temp_actual, &beer_pid_out, &beer_temp_target,
  BEER_P_DEFAULT, BEER_I_DEFAULT, BEER_D_DEFAULT, PID::DIRECT);

PID chamberTempPID(&chamber_temp_actual, &chamber_pid_out, &chamber_target_temp,
    CHAMBER_P_DEFAULT, CHAMBER_I_DEFAULT, CHAMBER_D_DEFAULT, PID::DIRECT);

RELAYS myRelays;
STORAGE myStorage;
INDICATORS myIndicator;

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

// ----------------------------------
// Initialize and prime beer temp PID
// ----------------------------------
void initPID_loops(void) {
  // Init pid fields
  beerP = myStorage.beerP();
  beerI = myStorage.beerI();
  beerD = myStorage.beerD();

  chamberP = myStorage.chamberP();
  chamberI = myStorage.chamberI();
  chamberD = myStorage.chamberD();

  beerTempPID.SetTunings(beerP, beerI, beerD);
  chamberTempPID.SetTunings(chamberP, chamberI, chamberD);

  //beer_temp_target = DEFAULT_SP;
  beer_temp_target = myStorage.beer_temp_target();
  chamber_target_temp = beer_temp_target;

  // use dummy values for now, they'll update first time
  // we read sensors
  beer_temp_actual = beer_temp_target; // 60.0; // 5/7/2020
  chamber_temp_actual = beer_temp_target; // 60.0;

  // Turn on pid
  beerTempPID.SetMode(PID::AUTOMATIC);
  beerTempPID.SetSampleTime(BEER_SAMPLETIME*1000);

  chamberTempPID.SetMode(PID::AUTOMATIC);
  chamberTempPID.SetSampleTime(CHAMBER_SAMPLETIME*1000);

  control_signal = 50;

  adjustChamberTempLimits(beer_temp_target);

  // beerTempPID.Initialize(beer_temp_actual);
  beerTempPID.Initialize(beer_temp_target); // 5/7/2020
  chamberTempPID.Initialize(50.0); // this isn't a temp but a control signal

  delay(30*1000);
  run_calculations();
  update_status();
}

/* ---------------------------------------------------------------------------*/
// Do initial network setup
/* ---------------------------------------------------------------------------*/
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

void checkNetworking(void) {
  if (!Particle.connected()) {
    WiFi.on();
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
// Read sensors to refresh actual temperature values
/* ---------------------------------------------------------------------------*/
void read_sensors() {

  mySensors.refresh();

    // Apply low pass filters to the temperature sensors to eliminate
  // noise, the temps change slowly so we're Okay with a slow change
  // rate on the temp readings. Chamber changes quicker than beer, so
  // it gets a high alpha parameter.

  chamber_temp_actual += 0.99 * (mySensors.GetTempF(SENSORS::CHAMBER) -
                                  chamber_temp_actual);

  beer_temp_actual += 0.99 * (mySensors.GetTempF(SENSORS::BEER) -
                               beer_temp_actual);

#if Wiring_BLE
    check_bluetooth();
#endif
}

/* ---------------------------------------------------------------------------*/
// Temps change slowly so we don't want to feed PID to frequently
/* ---------------------------------------------------------------------------*/
void SetPace(void) {
  ts_next_loop = ts_last_loop + MIN_LOOP_TIME;
  ts_now = Time.now();

  if (ts_now < ts_next_loop) {
    delay((ts_next_loop - ts_now) * 1000);
  }

  ts_last_loop = Time.now();
}

/* ---------------------------------------------------------------------------*/
//
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
    target_action = "Paus";
  } else if (control_signal < cool_limit) {
    myRelays.coolON();
    heat_cool_status = COOLING_ON;
    target_action = "Cool";
  } else if (control_signal > heat_limit) {
    myRelays.heatON();
    heat_cool_status = HEATER_ON;
    target_action = "Heat";
  } else {
    myRelays.idle();
    heat_cool_status = IDLE;
    target_action = "Idle";
  }

  switch (myRelays.getHeatCoolStatus()) {
    case COOLING_ON:
      current_action = "Cool";
      break;
    case HEATER_ON:
      current_action = "Heat";
      break;
    case IDLE:
      if (paused) {
        current_action = "Paus";
      } else {
        current_action = "Idle";
      }
      break;
  }

}

/* ---------------------------------------------------------------------------*/
// Cloud function to set beerPID 'P' value
/* ---------------------------------------------------------------------------*/
int setBeerP(String temp_P) {
    int result = 0;
    float temp = atof(temp_P); // returns 0.0 if atof fails

    if (temp > 0.0) {
      beerP = temp;
      beerTempPID.SetTunings(beerP, beerI, beerD);
      beerTempPID.SynchITerm();

      myStorage.store_beer_pid(beerP, beerI, beerD);
    } else {
      result = -1;
    }

    return result;
}

/* ---------------------------------------------------------------------------*/
// Cloud function to set beerPID 'I' value
/* ---------------------------------------------------------------------------*/
int setBeerI(String temp_I) {
    int result = 0;
    float temp = atof(temp_I); // returns 0.0 if atof fails

    if (temp > 0.0) {
      beerI = temp;
      beerTempPID.SetTunings(beerP, beerI, beerD);
      beerTempPID.SynchITerm();

      myStorage.store_beer_pid(beerP, beerI, beerD);
    } else {
      result = -1;
    }

    return result;
}

/* ---------------------------------------------------------------------------*/
// Cloud function to set beerPID 'D' value
/* ---------------------------------------------------------------------------*/
int setBeerD(String temp_D) {
    int result = 0;
    float temp = atof(temp_D); // returns 0.0 if atof fails

    if (temp > 0.0) {
      beerD = temp;
      beerTempPID.SetTunings(beerP, beerI, beerD);
      beerTempPID.SynchITerm();

      myStorage.store_beer_pid(beerP, beerI, beerD);
    } else {
      result = -1;
    }

    return result;
}


/* ---------------------------------------------------------------------------*/
// Cloud function to set chamberPID 'P' value
/* ---------------------------------------------------------------------------*/
int setChamberP(String temp_P) {
    int result = 0;
    float temp = atof(temp_P); // returns 0.0 if atof fails

    if (temp > 0.0) {
      chamberP = temp;
      chamberTempPID.SetTunings(chamberP, chamberI, chamberD);
      chamberTempPID.SynchITerm();

      myStorage.store_chamber_pid(chamberP, chamberI, chamberD);
    } else {
      result = -1;
    }

    return result;
}

/* ---------------------------------------------------------------------------*/
// Cloud function to set chamberPID 'I' value
/* ---------------------------------------------------------------------------*/
int setChamberI(String temp_I) {
    int result = 0;
    float temp = atof(temp_I); // returns 0.0 if atof fails

    if (temp > 0.0) {
      chamberI = temp;
      chamberTempPID.SetTunings(chamberP, chamberI, chamberD);
      chamberTempPID.SynchITerm();

      myStorage.store_chamber_pid(chamberP, chamberI, chamberD);
    } else {
      result = -1;
    }

    return result;
}

/* ---------------------------------------------------------------------------*/
// Cloud function to set chamberPID 'D' value
/* ---------------------------------------------------------------------------*/
int setChamberD(String temp_D) {
    int result = 0;
    float temp = atof(temp_D); // returns 0.0 if atof fails

    if (temp > 0.0) {
      chamberD = temp;
      chamberTempPID.SetTunings(chamberP, chamberI, chamberD);
      chamberTempPID.SynchITerm();

      myStorage.store_chamber_pid(chamberP, chamberI, chamberD);
    } else {
      result = -1;
    }

    return result;
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
void update_status() {

  // start with system_status ----
  // get times in seconds to calc work
  uptime = System.uptime();
  cooltime = myRelays.get_cool_runtime();
  heattime = myRelays.get_heat_runtime();

  percent_cool = (cooltime / uptime) * 100.0;
  percent_heat = (heattime / uptime) * 100.0;

  char buffer[MAX_DATA_SIZE];
  memset(buffer, 0, sizeof(buffer));

  snprintf(buffer, sizeof(buffer),
    "|BT:%4.1f|BA:%4.1f|CT:%4.1f|CA:%4.1f|CS:%4.1f|PH:%4.1f|PC:%4.1f|BI:%4.1f|CI:%4.1f|SG:%5.3f|end",
      beer_temp_target,
      beer_temp_actual,
      chamber_target_temp,
      chamber_temp_actual,
      chamber_pid_out,
      percent_heat,
      percent_cool,
      beerTempPID.GetITerm(),
      chamberTempPID.GetITerm(),
      tilt_sg
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

  // reset and do tuning status
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer),
    "|BP:%4.2f|BI:%1.4f|BD:%1.4f|CP:%4.2f|CI:%1.4f|CD:%1.4f|end",
      beerP,
      beerI,
      beerD,
      chamberP,
      chamberI,
      chamberD
      );

  system_tuning.remove(0);
  system_tuning.concat(buffer);

} // end of update_status ----------------------------------------


#if Wiring_BLE
/* example packet
0 - 4C: manufacturer ID - Apple iBeacon
1 - 00: manufacturer ID - Apple iBeacon
2 - 02: type (constant, defined by iBeacon spec)
3 - 15: length (constant, defined by iBeacon spec)
4 - A4: device UUID
5 - 95: device UUID
6 - BB: device UUID
7 - 10: device UUID
8 - C5: device UUID
9 - B1: device UUID
10- 4B: device UUID
11- 44: device UUID
12- B5: device UUID
13- 12: device UUID
14- 13: device UUID
15- 70: device UUID
16- F0: device UUID
17- 2D: device UUID
18- 74: device UUID
19- DE: device UUID
20- 00: major - temperature (in degrees fahrenheit)
21- 44: major - temperature (in degress fahrenheit)
22- 03: minor - specific gravity (x1000)
23- F8: minor - specific gravity (x1000)
24- C5: TX power in dBm
25- C7: RSSI in dBm
*/
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void check_bluetooth() {
    ble_device_count = BLE.scan(scanResults, SCAN_RESULT_MAX);
    if (ble_device_count > ble_max_count) {
      ble_max_count = ble_device_count;
    }

    for (int ii = 0; ii < ble_device_count; ii++) {
      uint8_t buf[BLE_MAX_ADV_DATA_LEN];
      size_t len;

      len = scanResults[ii].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);

      if (len == 25) {
        String head = String::format("%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3]);
        if (head == "4C000215") { // found an iBeacon
          String uuid = String::format("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
          buf[4], buf[5], buf[6], // skip buf[7] - it says what color of tilt
          buf[8], buf[9], buf[10], buf[11],
          buf[12], buf[13], buf[14], buf[15],
          buf[16], buf[17], buf[18], buf[19]);

          if (uuid == "A495BBC5B14B44B5121370F02D74DE") { // we found a tilt
            beaconMajor = String::format("Temp = %d", (buf[20] * 256) + buf[21]);
            tilt_sg = ((buf[22] * 256) + buf[23]) / 1000.0;
          }
        }
      }
    }
}
#endif

//---------------------------------------------------------------------------
// setup() runs once, when the device is first turned on.
//---------------------------------------------------------------------------
void setup() {
  Particle.variable("SystemStatus", system_status);
  Particle.variable("TuningStatus", system_tuning);
  Particle.variable("Uptime", uptime);

  Particle.function("setBeerTarget", setBeerTarget);
  Particle.function("setPauseState", setPause);

  Particle.function("setBeerP", setBeerP);
  Particle.function("setBeerI", setBeerI);
  Particle.function("setBeerD", setBeerD);

  Particle.function("setChamberP", setChamberP);
  Particle.function("setChamberI", setChamberI);
  Particle.function("setChamberD", setChamberD);

  // Always expose these, just may not be valid values
  Particle.variable("bleDeviceCount", ble_max_count);
  Particle.variable("beaconMajor", beaconMajor);

  // Just to see what's up
  Serial.begin(9600);
  delay(10000); // give time to connect to tty
  Serial.println("opening");
  delay(500);

  checkNetworking();
  initPID_loops();

  paused = myStorage.pause_state();

  // these are fake values for test when no sensors attached
  fake_beer_temp = 64.7;
  fake_chamber_temp = 70.0;

  cool_limit = COOL_LIMIT2;
  heat_limit = HEAT_LIMIT2;

  ts_lastSim = Time.now();
  ts_last_loop = ts_lastSim - 200; // pretend we last looped 200 seconds ago
  ts_last_publish = ts_last_loop;

  Serial.println("Getting ready to init sensors");
  delay(500);
  mySensors.init();
  Serial.println("After init, start sensor refresh");
  delay(500);
  mySensors.refresh();
  Serial.println("After refresh");
  delay(500);

#if Wiring_BLE
    BLE.on();
#endif

  // Start with relays off
  //TODO: replace pin numbers with the defined versions
  pinMode(D2, OUTPUT); // end
  digitalWrite(D2, LOW);

  pinMode(D7, OUTPUT);
  digitalWrite(D7, LOW); // middle
}

//---------------------------------------------------------------------------
// loop() runs over and over again, as quickly as it can execute.
//---------------------------------------------------------------------------
void loop() {
    checkNetworking();
    Particle.process();

    read_sensors();  // reads the sensosrs to get temps
    Particle.process();

    if (!paused) {
      run_calculations();
      Particle.process();
      beerITerm = beerTempPID.GetITerm();
      chamberITerm = chamberTempPID.GetITerm();
    }

    control_HeatCool();
    Particle.process();

    update_status();
    Particle.process();

    myIndicator.setStatus(beer_temp_actual - beer_temp_target);

    SetPace();
}
