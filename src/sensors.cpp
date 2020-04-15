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

DS18 sensor(D2);  // define which pin have the sensors connected

SENSORS::SENSORS(void) {
  beerF = UNKNOWN_TEMP;
  chamberF = UNKNOWN_TEMP;
  ambientF = UNKNOWN_TEMP;

  refresh();
}

void SENSORS::refresh(void) {
  // Read the next available 1-Wire temperature sensor
  uint8_t addr[8];
  char buffer[BUFF_SIZE];

  sensor.read();
  while (!sensor.searchDone()) {
    if (!sensor.crcError()) {
      sensor.addr(addr); // get the address for the temp value

      // convert address to hex string
      snprintf(buffer, BUFF_SIZE,
        "%02X%02X%02X%02X%02X%02X%02X%02X",
        addr[0], addr[1], addr[2], addr[3],
        addr[4], addr[5], addr[6], addr[7]);

      if (strcmp(types[BEER], buffer) == 0) {
        beerF = sensor.fahrenheit() + beerF_calibrate;
      } else if (strcmp(types[CHAMBER], buffer) == 0) {
        chamberF = sensor.fahrenheit() + chamberF_calibrate;
      } else if (strcmp(types[AMBIENT], buffer) == 0) {
        ambientF = sensor.fahrenheit();
      }
    }  // !crcError

    delay(250);  // need time between reads
    sensor.read();
  }

}

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
