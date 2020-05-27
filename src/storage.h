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

#ifndef STORAGE_H_
#define STORAGE_H_

// Includes ---- photon vendor defined ----
#include "application.h"

// Includes ---- project defined ----
#include "brewferm.h"

#define STORAGE_DEF_NAME "BrewFerm\0"

class STORAGE {
 public:
  // commonly used functions
    void store_beer_temp_target(double sp);
    void store_pause_state(boolean state);
    void store_beer_pid(double p, double i, double d);
    void store_chamber_pid(double p, double i, double d);

    double beer_temp_target();
    boolean pause_state();

    double beerP();
    double beerI();
    double beerD();

    double chamberP();
    double chamberI();
    double chamberD();

    void getBeerSensorAddr(uint8_t addr[8]);
    void getChamberSensorAddr(uint8_t addr[8]);

    void store_beer_sensor_addr(uint8_t addr[8]);
    void store_chamber_sensor_addr(uint8_t addr[8]);

  private:
    void init(void);
    boolean init_ran = false;

  // stored values
    double beer_target = 64.0; //TODO: get default from brewferm.h

    boolean paused = true;
};

#endif  // STORAGE_H_
