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

// Controls the sensors
#include "brewferm.h"
#include "sensors.h"

#include <string>

#define BUFF_SIZE 2048
#define MAX_SENSORS 6

int num_sensors;
retained uint8_t sensorAddresses[MAX_SENSORS][8];
float celsius[MAX_SENSORS] = {NAN, NAN, NAN, NAN, NAN, NAN};

// #############################
const int16_t dsData = A4; // D3;
DS18B20 sensor(dsData);
// #############################

//---------------------------------------------------------------------------
// We haven't established the sensors or temps at this point
//---------------------------------------------------------------------------
SENSORS::SENSORS(void) {
  beerF = UNKNOWN_TEMP;
  chamberF = UNKNOWN_TEMP;
  ambientF = UNKNOWN_TEMP;
}

//---------------------------------------------------------------------------
//TODO: change to detect how many sensors connected instead of assuming two
//---------------------------------------------------------------------------
void SENSORS::init(void) {
  sensor.resetsearch();                 // initialise for sensor search

  boolean found = false;
  int i = 0;

  do {
    found = sensor.search(sensorAddresses[i++]); // and if available store
    if (found) {
      num_sensors = i;
    }
  } while ((found == true) && (i < MAX_SENSORS));

}

const int MAXRETRY = 3;

double getTemp(uint8_t addr[8]) {
  double _temp;
  int   i = 0;

  do {
    _temp = sensor.getTemperature(addr);
    if (!sensor.crcCheck()) {
      delay(200); // wait and try again
    }
  } while ((!sensor.crcCheck()) && (MAXRETRY > i++));

  if (i < MAXRETRY) {
    Serial.printf("address = %02X %02X %02X %02X %02X %02X %02X %02X  =  ",
        addr[0], addr[1], addr[2], addr[3],
        addr[4], addr[5], addr[6], addr[7]);
    Serial.println(_temp);
  }
  else {
    _temp = NAN;
    Serial.println("Invalid reading");
  }

  return _temp;
}

//---------------------------------------------------------------------------
// loop through sensors found in init() and store retrieved temps
//---------------------------------------------------------------------------
void SENSORS::refresh(void) {
  // Read the next available 1-Wire temperature sensor

  char buffer[BUFF_SIZE];

  for (int i = 0; i < num_sensors; i++) {
    float temp = getTemp(sensorAddresses[i]);

    if (!isnan(temp)) {
      celsius[i] = temp;

      // convert address to hex string
      snprintf(buffer, BUFF_SIZE,
        "%02X%02X%02X%02X%02X%02X%02X%02X",
        sensorAddresses[i][0], sensorAddresses[i][1],
        sensorAddresses[i][2], sensorAddresses[i][3],
        sensorAddresses[i][4], sensorAddresses[i][5],
        sensorAddresses[i][6], sensorAddresses[i][7]);

      if (strcmp(types[BEER], buffer) == 0) {
        beerF = sensor.convertToFahrenheit(temp) + beerF_calibrate;
      } else if (strcmp(types[CHAMBER], buffer) == 0) {
        chamberF = sensor.convertToFahrenheit(temp) + chamberF_calibrate;
      } else if (strcmp(types[AMBIENT], buffer) == 0) {
        ambientF = sensor.convertToFahrenheit(temp);
      }
    }
    delay(200); // give plenty of time for sensors to reset
  }
}

//---------------------------------------------------------------------------
// This returns the last temp found in the refresh() function
//---------------------------------------------------------------------------
double SENSORS::GetTempF(sensor_use mode) {
  if (mode == BEER) {
    return beerF;
  } else if (mode == CHAMBER) {
    return chamberF;
  } else if (mode == AMBIENT) {
    return ambientF;
  }

  return UNKNOWN_TEMP;
}
