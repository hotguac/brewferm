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

/* Sensor pin outs

viewed from front of sensor RJ14 plug with tab UP

pin 1 - no connection
pin 2 - Red - 5V
pin 3 - Black - Ground
pin 4 - White - data

viewed from front of wall RJ25 wall jack with tab DOWN

pin 1 -
pin 2 - black
pin 3 - red
pin 4 - green
pin 5 - yellow
pin 6 -

connection mapping from sensor plug to jack

Red 5V -> Red
Black Ground -> green
White Data -> yellow

*/

#include "DS18B20.h"

#ifndef SENSORS_H_
#define SENSORS_H_

#define MAX_SENSORS 4
#define ADDRESS_SIZE (sizeof(uint8_t) * 8)

class SENSORS {
    public:
        const double beerF_calibrate = -1.9;
        const double chamberF_calibrate = -1.5;

        SENSORS(void);
        void init();

        void wait_for_no_sensors();
        boolean beer_sensor_attached(uint8_t beerAddress[8]);
        boolean chamber_sensor_attached(uint8_t chamberAddress[8]);
        boolean ambient_sensor_attached(void);

        boolean assign_beer_sensor(uint8_t addr[8]);
        boolean assign_chamber_sensor(uint8_t addr[8]);
        boolean assign_ambient_sensor(uint8_t addr[8]);

        void refresh(void);
        double GetBeerF();
        double GetChamberF();
        double GetAmbientF();

    private:
        float beerF = NAN;
        float chamberF = NAN;
        float ambientF = NAN;

        int num_sensors = 0;

        uint8_t sensorBeer[8]; // sensor assigned to beer
        uint8_t sensorChamber[8]; // sensor assigned to chamber
        uint8_t sensorAmbient[8]; // sensor assigned to ambient

        uint8_t sensorAddresses[MAX_SENSORS][8]; // all detected sensors

        double getTemp(uint8_t addr[8]);
};

#endif  // SENSORS_H_
