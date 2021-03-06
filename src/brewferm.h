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

#define SENSOR_PIN A0
#define HEAT_PIN D2
#define COOL_PIN D4
#define HEARTBEAT_PIN D7 // This is the built-in 'user' single color LED

// Controls how frequently the process loop runs
// We slow it down because the temps don't change quickly
#define MIN_LOOP_TIME 12 // seconds
#define BEER_SAMPLETIME 120 // Compute interval in seconds
#define CHAMBER_SAMPLETIME 24 // Compute interval in seconds
#define SEND_STATUS_INTERVAL 1200 // 1200 // in seconds

// The Beer PID parameters; known good values P 0.6; I 0.0005; D 1.0
#define BEER_P_DEFAULT  3.2 // 2.6
#define BEER_I_DEFAULT  0.00010 // 0.00007
#define BEER_D_DEFAULT  0.0 // 1.0

// The Chamber PID parameters; known good values P 4; I 0.04; D 1.25
#define CHAMBER_P_DEFAULT  4 // 4
#define CHAMBER_I_DEFAULT  0.01 // 0.04
#define CHAMBER_D_DEFAULT  0.0 // 1.25

// Tempature control parameters
// These work with the output of the chamber PID controller
// On a 1-99 range with 1 being full make it colder and
// 99 being full make it hotter
#define COOL_LIMIT1  18 // 4
#define COOL_LIMIT2  20 // 10
#define HEAT_LIMIT2  60
#define HEAT_LIMIT1  80 // 96

#define MIN_HEAT_COOL    120   // seconds between turning heat off and cool on
#define MIN_COOL_HEAT    120   // seconds between turning cool off and heat on
#define MIN_HEAT_OFF_ON  30    // seconds between turning heat off and heat on

#define MIN_COOL_OFF_ON  420  // seconds between turning cool off and cool on
                              // WARNING!!! this is a safety feature and shouldn't
                              // be less than 6 minutes (360 seconds) to keep from
                              // causing compressor damage; should only be
                              // changed if you know what you're doing.

#define MIN_COOL_TIME     30  // minimum seconds between cool on and cool off
#define MIN_HEAT_TIME     15  // minimum seconds between heat on and heat off

#define MAX_COOL_TIME    360  // seconds between cool on and cool off
#define MAX_HEAT_TIME    360  // seconds between heat on and heat off

// Other PID algorithm parameters
#define MIN_OUTPUT_TEMP 31.0  // minimum chamber target temperature
#define LIMIT_RANGE_LOW 6.0   // how much colder chamber target can be from beer target (unless crashing)
#define LIMIT_RANGE_HIGH 4.0  // how much warmer chamber target can be from beer target

// Parameters dealing with the Set Point
#define MIN_SP  32.9  // minimum beer target temp
#define MAX_SP  80.0  // maximum beer target temp
#define DEFAULT_SP  66.0 // beer target on reboot if we don't have a stored value

#define FERM_CRASH_SPLIT 40 // if beer target is less than this, we're cold crashing
                            // so allow chamber temp to drop to MIN_OUTPUT_TEMP

// Sensor parameters
#define UNKNOWN_TEMP -9.9

// Heat / Cool sendStatus
enum mode_heat_cool { COOLING_ON = 1,
                      IDLE = 2,
                      HEATER_ON = 3 };

#endif  // BREWFERM_H_
