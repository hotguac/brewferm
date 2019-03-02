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

// STORAGE::STORAGE(void) {
// }

void STORAGE::store_beer_temp_target(double sp) {
  if ((sp > MIN_SP) && (sp < MAX_SP)) {
      //snprintf(mySP.name, EEPROM_CHECK_SIZE, "beerPID_Setpoint");
      strcpy("12345678\0",mySP.name);
      //mySP.name = "12345678\0";
      mySP.value = sp;
      EEPROM.put(EEPROM_SP_ADDR, mySP);
    }
}

double STORAGE::retrieve_beer_temp_target() {
  delay(10*1000);
  // Serial.printf("Before EEPROM get\r\n");

  EEPROM.get(EEPROM_SP_ADDR, mySP);

  if (strcmp(mySP.name, "12345678\0") != 0) {
    if ((mySP.value < MIN_SP) || (mySP.value > MAX_SP)) {
      mySP.value = DEFAULT_SP;
      store_beer_temp_target(mySP.value);
    }
  }

// until i figure it beerPID_Output
  // return 65.0;
  return mySP.value;
}
