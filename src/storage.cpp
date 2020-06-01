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
    uint32_t type;

    double beer_target;

    double beerP;
    double beerI;
    double beerD;

    double chamberP;
    double chamberI;
    double chamberD;

    boolean paused;

    uint8_t sensorAddressBeer[8];
    uint8_t sensorAddressChamber[8];
};

struct FermSettings0x11 {
    uint32_t type;

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

#define CURRENT_VERSION 0x12
#define X11_VERSION 0x11 // this is a previous version of settings

//----------------------------------------------------------------------------
// Read settings and check the type, copy and update and necesary
//----------------------------------------------------------------------------
void STORAGE::init(void) {
    init_ran = true;

    EEPROM.get(0, settings);

    switch (settings.type) {
        case CURRENT_VERSION:
            break;
        case X11_VERSION:
            FermSettings0x11 settingsX11;
            EEPROM.get(0, settingsX11);

            settings.type = CURRENT_VERSION;
            settings.beer_target = settingsX11.beer_target;

            settings.beerP = settingsX11.beerP;
            settings.beerI = settingsX11.beerI;
            settings.beerD = settingsX11.beerD;

            settings.chamberP = settingsX11.chamberP;
            settings.chamberI = settingsX11.chamberI;
            settings.chamberD = settingsX11.chamberD;

            settings.paused = settingsX11.paused;

            memset(settings.sensorAddressBeer, 0, sizeof(uint8_t) * 8);
            memset(settings.sensorAddressChamber, 0, sizeof(uint8_t) * 8);

            break;
    default:
        settings.type = CURRENT_VERSION;
        beer_target = settings.beer_target;

        settings.beer_target = 58.0;
        settings.paused = true;

        settings.beerP = BEER_P_DEFAULT;
        settings.beerI = BEER_I_DEFAULT;
        settings.beerD = BEER_D_DEFAULT;

        settings.chamberP = CHAMBER_P_DEFAULT;
        settings.chamberI = CHAMBER_I_DEFAULT;
        settings.chamberD = CHAMBER_D_DEFAULT;

        memset(settings.sensorAddressBeer, 0, sizeof(uint8_t) * 8);
        memset(settings.sensorAddressChamber, 0, sizeof(uint8_t) * 8);

        paused = settings.paused;
        break;
    }

    paused = settings.paused;
    beer_target = settings.beer_target;

    EEPROM.put(0, settings); // if no change, no actual write occurs
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_beer_sensor_addr(uint8_t addr[8]) {
    if (init_ran == false) { init(); }
    memcpy(settings.sensorAddressBeer, addr, sizeof(uint8_t) * 8);
    EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_chamber_sensor_addr(uint8_t addr[8]) {
    if (init_ran == false) { init(); }
    memcpy(settings.sensorAddressChamber, addr, sizeof(uint8_t) * 8);
    EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_beer_temp_target(double sp) {
    if (init_ran == false) { init(); }
    settings.beer_target = sp;
    EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::getBeerSensorAddr(uint8_t addr[8]) {
    memcpy(addr, settings.sensorAddressBeer, sizeof(uint8_t) * 8);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::getChamberSensorAddr(uint8_t addr[8]) {
    memcpy(addr, settings.sensorAddressChamber, sizeof(uint8_t) * 8);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_pause_state(boolean state) {
    if (init_ran == false) { init(); }
    settings.paused = state;
    EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_beer_pid(double p, double i, double d) {
    if (init_ran == false) { init(); }

    settings.beerP = p;
    settings.beerI = i;
    settings.beerD = d;
    settings.type = CURRENT_VERSION;

    EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void STORAGE::store_chamber_pid(double p, double i, double d) {
    if (init_ran == false) { init(); }

    settings.chamberP = p;
    settings.chamberI = i;
    settings.chamberD = d;
    settings.type = CURRENT_VERSION;

    EEPROM.put(0, settings);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::beer_temp_target() {
    if (init_ran == false) { init(); }
    return beer_target;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
boolean STORAGE::pause_state() {
    if (init_ran == false) { init(); }
    return settings.paused;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::beerP() {
    if (init_ran == false) { init(); }
    return settings.beerP;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::beerI() {
    if (init_ran == false) { init(); }
    return settings.beerI;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::beerD() {
    if (init_ran == false) { init(); }
    return settings.beerD;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::chamberP() {
    if (init_ran == false) { init(); }
    return settings.chamberP;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::chamberI() {
    if (init_ran == false) { init(); }
    return settings.chamberI;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double STORAGE::chamberD() {
    if (init_ran == false) { init(); }
    return settings.chamberD;
}
