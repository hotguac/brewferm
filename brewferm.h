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

#ifndef BREWFERM_H_
#define BREWFERM_H_

// Uncomment this define to compile with simulated temp Sensors
#define RUN_SIMULATION 1

// we call a delay because it let's the system routines run, such as
//   wifi calls, and because temperature doesn't change as fast as the
//   device could run.
#define DELAY_TIME 100  // milliseconds

// What pins are wired to what
#define LED_PIN   D7
#define GREEN_PIN D0
#define RED_PIN   D1

// The PID parameters
#define BEER_P  4  // 8
#define BEER_I  0.125
#define BEER_D  120  // 80

#define CHAMBER_P  6 // 4
#define CHAMBER_I  0.5
#define CHAMBER_D  80

// Tempature control parameters
// These work with the output of the chamber PID controller
// On a 1-99 range with 1 being full make it colder and
// 99 being full make it hotter
#define COOL_LIMIT1  6
#define COOL_LIMIT2  30
#define HEAT_LIMIT2  60
#define HEAT_LIMIT1  96

#define MIN_HEAT_COOL    65
#define MIN_COOL_HEAT    65
#define MIN_HEAT_OFF_ON  120   // seconds
#define MIN_COOL_OFF_ON  360  // this is a safety feature and shouldn't
                              // be less than 6 minutes (360 seconds) to keep
                              // the compressor for damage; should only be
                              // changed if you know what you're doing.

#define MIN_COOL_TIME    90  // seconds
#define MIN_HEAT_TIME    65  // seconds

#define MAX_COOL_TIME    240  // seconds
#define MAX_HEAT_TIME    480  // seconds

// Other PID algorithm parameters
#define MIN_OUTPUT_TEMP 31.0
#define LIMIT_RANGE_LOW 6.0
#define LIMIT_RANGE_HIGH 4.0

// Parameters dealing with the Set Point
#define MIN_SP  32.9
#define MAX_SP  80.0
#define DEFAULT_SP  66.0
#define FERM_CRASH_SPLIT 40

// Indicator LED parameters
#define INDICATE_OK     0.3
#define INDICATE_CLOSE  0.6

// EEPROM addresses
#define EEPROM_SP_ADDR  0
#define EEPROM_CHECK_SIZE  9

// Serial Port config
#define SERIAL_BAUD 9600

// Communication parameters
#define TRANSMIT_INTERVAL 60

// UDP parameters
#define REMOTE_PORT  7777
#define LOCAL_PORT   8888

// Sensor parameters
#define UNKNOWN_TEMP -9.9

// Heat / Cool sendStatus
enum mode_heat_cool { COOLING_ON = 1,
                      IDLE = 2,
                      HEATER_ON = 3 };

#endif  // BREWFERM_H_
