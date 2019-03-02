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
#include "Sensors.h"

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
      sensor.addr(addr);
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

    delay(250);
    sensor.read();
  }

  // Once all sensors have been read you'll get searchDone() == true
  // Next time read() is called the first sensor is read again
  if (!sensor.searchDone()) {
    // Serial.println("Hmmmmm...");
  }  // !sensor.searchDone()
}

void SENSORS::printDebugInfo() {
  // If there's an electrical error on the 1-Wire bus you'll get a CRC error
  // Just ignore the temperature measurement and try again
  if (sensor.crcError()) {
    // Serial.print("CRC Error ");
  }

  // Print the sensor type
  #define TYPE_SIZE 10
  char type[TYPE_SIZE];
  switch (sensor.type()) {
    case WIRE_DS1820: snprintf(type, TYPE_SIZE, "DS1820"); break;
    case WIRE_DS18B20: snprintf(type, TYPE_SIZE, "DS18B20"); break;
    case WIRE_DS1822: snprintf(type, TYPE_SIZE, "DS1822"); break;
    case WIRE_DS2438: snprintf(type, TYPE_SIZE, "DS2438"); break;
    default: snprintf(type, TYPE_SIZE, "UNKNOWN"); break;
  }
  // Serial.print(type);

  // Print the ROM (sensor type and unique ID)
  uint8_t addr[8];
  sensor.addr(addr);
  // Serial.printf(
  //  " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
  //  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  //);

  // Print the raw sensor data
  uint8_t data[9];
  sensor.data(data);
  // Serial.printf(
  //  " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
  //  data[0], data[1], data[2], data[3],
  //  data[4], data[5], data[6], data[7], data[8]
  //);
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
