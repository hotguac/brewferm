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

#ifndef RELAYS_H_
#define RELAYS_H_

#include <application.h>

#include "brewferm.h"

class RELAYS {
 public:
  // Parameter types for some of the functions below
    const char *types[3] = {"2875857F08000063",
                            "2874AD7F08000092",
                            "0000000000000000"};

    RELAYS(void);
    void heatON(void);
    void idle(void);
    void coolON(void);

    mode_heat_cool getHeatCoolStatus();
    unsigned int get_heat_runtime();
    unsigned int get_cool_runtime();

 private:
    int relayCoolPin = D2;
    int relayHeatPin = D7;


    // these are in seconds
    unsigned int heat_runtime;
    unsigned int cool_runtime;

    // run at least this long
    double min_cool_off_on = MIN_COOL_OFF_ON;

    mode_heat_cool heat_cool_Status;

    time_t ts_heatON;
    time_t ts_heatOFF;
    time_t ts_coolON;
    time_t ts_coolOFF;
};

#endif
