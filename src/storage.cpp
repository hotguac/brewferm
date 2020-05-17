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

#include "storage.h"

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::init(void) {
  init_ran = true;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_beer_temp_target(double sp) {
    EEPROM.put(0, sp);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_pause_state(boolean state) {
  EEPROM.put(16, state);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::beer_temp_target() {
  if (!init_ran) {
    init();
  }

  return beer_target;

/* old code
  double temp;
  EEPROM.get(0, temp);

  if ((temp < MIN_SP) || (temp > MAX_SP)) {
      temp = DEFAULT_SP;
      store_beer_temp_target(temp);
  }

  return temp;
*/
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
boolean STORAGE::pause_state() {
  if (!init_ran) {
    init();
  }

  return beer_target;

/* old code
  boolean state;

  EEPROM.get(16, state);

  return state;
*/
}
