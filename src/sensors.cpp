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

//#define BUFF_SIZE 2048

// #############################
const int16_t dsData = SENSOR_PIN;
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
//
// This routine scans for attached sensors and stores their addresses.
//---------------------------------------------------------------------------
void SENSORS::init() {
    Serial.print("starting sensor init ");

    sensor.resetsearch();                 // initialise for sensor search

    boolean found = false;
    int i = 0;
    num_sensors = 0;

    do {
        found = sensor.search(sensorAddresses[i++]); // and if available store
        if (found) {
            num_sensors = i;
        }
    } while ((found == true) && (i < MAX_SENSORS));

    Serial.printlnf("found %d sensors", num_sensors);

    Particle.process();
}

//---------------------------------------------------------------------------
// This routine waits for a new sensor to show up in the scan and
// then assigns it the role of beer sensor and updates the addresss.
//---------------------------------------------------------------------------
boolean SENSORS::assign_beer_sensor(uint8_t addr[8]) {
    boolean found = false;

    int try_count = 20;
    int i = 0;

    Serial.println("starting check for beer sensor");

    do {
        init(); //scans for attached sensors

        if (num_sensors == 1) {
            memcpy(sensorBeer, sensorAddresses[0], sizeof(uint8_t) * 8);
            memcpy(addr, sensorAddresses[0], sizeof(uint8_t) * 8);
            found = true;
        } else {
            delay(5000);
            Serial.printlnf("... currently %d sensors attached", num_sensors);
            Serial.println("... waiting for beer sensor to be attached");
        }

    } while ((found == false) && (i < try_count));

    return found;
}

#define ADDRESS_SIZE (sizeof(uint8_t) * 8)
//---------------------------------------------------------------------------
// This routine waits for a new sensor to show up in the scan and
// then assigns it the role of beer sensor and updates the addresss.
//---------------------------------------------------------------------------
boolean SENSORS::assign_chamber_sensor(uint8_t addr[8]) {
    boolean found = false;

    int try_count = 20;
    int i = 0;

    Serial.println("---");
    Serial.println("starting check for chamber sensor");

    do {
        init(); //scans for attached sensors

        if (num_sensors == 2) { // beer sensor should already be configured
            if (memcmp(sensorBeer, sensorAddresses[0], ADDRESS_SIZE) == 0) {
                Serial.println("Sensor 0 is beer, assigning sensor 1");
                memcpy(sensorChamber, sensorAddresses[1], ADDRESS_SIZE);
                memcpy(addr, sensorAddresses[1], ADDRESS_SIZE);
                found = true;
            } else {
                memcpy(sensorChamber, sensorAddresses[0], ADDRESS_SIZE);
                memcpy(addr, sensorAddresses[0], ADDRESS_SIZE);
                found = true;
            }

        } else {
            delay(5000);
            Serial.println("... waiting for chamber sensor to be attached");
        }

    } while ((found == false) && (i++ < try_count));

    return found;
}

//---------------------------------------------------------------------------
// during the sensor assign process need to start with no sensors attached
//---------------------------------------------------------------------------
void SENSORS::wait_for_no_sensors() {
    boolean found = false;

    do {
        init();
        Serial.printlnf("Waiting for zero sensors, found %d", num_sensors);
        delay(2000);
    } while (num_sensors != 0);
}

//---------------------------------------------------------------------------
// The address of the sensor previously assigned to be the beer sensor is
// passed in. The function checks to see if that sensor responds to queries.
//---------------------------------------------------------------------------
boolean SENSORS::beer_sensor_attached(uint8_t addr[8]) {
    boolean found = false;

    Serial.print("Checking for beer sensor at addres ");
    Serial.printf(" %02X %02X %02X %02X %02X %02X %02X %02X  =  ",
        addr[0], addr[1], addr[2], addr[3],
        addr[4], addr[5], addr[6], addr[7]);

    memcpy(sensorBeer, addr, sizeof(uint8_t) * 8);

    if (!isnan(getTemp(sensorBeer))) {
        found = true;
    }

    return found;
}

//---------------------------------------------------------------------------
// Stores the passed in sensor address as the chamber sensor and then
// validates that it responds to queries
//---------------------------------------------------------------------------
boolean SENSORS::chamber_sensor_attached(uint8_t addr[8]) {
    boolean found = false;

    Serial.println("starting check for chamber sensor at address ");
    Serial.printf(" %02X %02X %02X %02X %02X %02X %02X %02X  =  ",
        addr[0], addr[1], addr[2], addr[3],
        addr[4], addr[5], addr[6], addr[7]);

    memcpy(sensorChamber, addr, sizeof(uint8_t) * 8);

    if (!isnan(getTemp(sensorBeer))) {
        found = true;
    }

    return found;
}

//---------------------------------------------------------------------------
// Query a specific tempature sensor address
//---------------------------------------------------------------------------
double SENSORS::getTemp(uint8_t addr[8]) {
    double _temp;
    const int MAXRETRY = 3;
    int i = 0;

    Serial.printf("address = %02X %02X %02X %02X %02X %02X %02X %02X  =  ",
        addr[0], addr[1], addr[2], addr[3],
        addr[4], addr[5], addr[6], addr[7]);

    do {
        _temp = sensor.getTemperature(addr);
        if (!sensor.crcCheck()) {
            delay(200); // wait and try again
        }
    } while ((!sensor.crcCheck()) && (MAXRETRY > i++));

    if (i < MAXRETRY) {
        Serial.println(_temp);
    }
    else {
        _temp = NAN;
        Serial.println("Invalid reading");
    }

    return _temp;
}

//---------------------------------------------------------------------------
// Query all sensors and store value
//---------------------------------------------------------------------------
void SENSORS::refresh(void) {
    Serial.println("Sensor refresh");

    float temp;
    temp = getTemp(sensorBeer);
    if (!isnan(temp)) {
        beerF = sensor.convertToFahrenheit(temp);
    }

    delay(200); // give plenty of time for sensors to reset

    temp = getTemp(sensorChamber);
    if (!isnan(temp)) {
        chamberF = sensor.convertToFahrenheit(temp);
    }

    delay(200); // give plenty of time for sensors to reset

    temp = getTemp(sensorAmbient);
    if (!isnan(temp)) {
        ambientF = sensor.convertToFahrenheit(temp);
    }

    Particle.process();
}

//---------------------------------------------------------------------------
// This returns the last temp found in the refresh() function
//---------------------------------------------------------------------------
double SENSORS::GetBeerF() {
    return beerF;
}

//---------------------------------------------------------------------------
// This returns the last temp found in the refresh() function
//---------------------------------------------------------------------------
double SENSORS::GetChamberF() {
    return chamberF;
}

//---------------------------------------------------------------------------
// This returns the last temp found in the refresh() function
//---------------------------------------------------------------------------
double SENSORS::GetAmbientF() {
    return ambientF;
}
