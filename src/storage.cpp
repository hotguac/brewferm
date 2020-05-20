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

struct FermSettings {
  uint8_t type;

  double beer_target;

  double beerP;
  double beerI;
  double beerD;

  double chamberP;
  double chamberI;
  double chamberD;

  boolean paused;
};

FermSettings settings;

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::init(void) {
  init_ran = true;

  EEPROM.get(0, settings);

  if (settings.type == 0x10) {
    beer_P_value = settings.beerP;
    beer_I_value = settings.beerI;
    beer_D_value = settings.beerD;

    chamber_P_value = settings.chamberP;
    chamber_I_value = settings.chamberI;
    chamber_D_value = settings.chamberD;

    paused = settings.paused;
    beer_target = settings.beer_target;
  } else {
    settings.type = 0x10;

    settings.beer_target = 64.0;
    settings.paused = true;

    settings.beerP = BEER_P_DEFAULT;
    settings.beerI = BEER_I_DEFAULT;
    settings.beerP = BEER_P_DEFAULT;

    settings.chamberP = CHAMBER_P_DEFAULT;
    settings.chamberI = CHAMBER_I_DEFAULT;
    settings.chamberD = CHAMBER_D_DEFAULT;

    EEPROM.put(0, settings);
  }

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_beer_temp_target(double sp) {
  if (!init_ran) { init(); }
  settings.beer_target = sp;
  EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_pause_state(boolean state) {
  if (!init_ran) { init(); }
  settings.paused = state;
  EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::beer_temp_target() {
  if (!init_ran) { init(); }
  return beer_target;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
boolean STORAGE::pause_state() {
  if (!init_ran) { init(); }
  return beer_target;
}

double STORAGE::beerP() {
  if (!init_ran) { init(); }
  return beer_P_value;
}

double STORAGE::beerI() {
  if (!init_ran) { init(); }
  return beer_I_value;
}

double STORAGE::beerD() {
  if (!init_ran) { init(); }
  return beer_D_value;
}

double STORAGE::chamberP() {
  if (!init_ran) { init(); }
  return chamber_P_value;
}

double STORAGE::chamberI() {
  if (!init_ran) { init(); }
  return chamber_I_value;
}

double STORAGE::chamberD() {
  if (!init_ran) { init(); }
  return chamber_D_value;
}
