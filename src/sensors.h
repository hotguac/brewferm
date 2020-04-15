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

#ifndef SENSORS_H_
#define SENSORS_H_

#include "DS18.h"

class SENSORS {
 public:
    // Parameter types for some of the functions below
    enum sensor_use { BEER = 0, CHAMBER = 1, AMBIENT = 2 };
    const char *types[3] = {"2875857F08000063",
                            "2874AD7F08000092",
                            "0000000000000000"};

    const double beerF_calibrate = -1.9;
    const double chamberF_calibrate = -1.5;

    SENSORS(void);
    void refresh(void);


    sensor_use GetMode(void);

    void SetSampleTime(int);

    double GetID(sensor_use);
    double GetTempF(sensor_use);

 private:
    void Initialize();
    float beerF;
    float chamberF;
    float ambientF;


};

#endif  // SENSORS_H_
