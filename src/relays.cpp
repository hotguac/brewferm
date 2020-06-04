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

#include "relays.h"

//----------------------------------------------------------------------------
// Used to control turning on and off the relays controlling heat and cooling
//----------------------------------------------------------------------------
RELAYS::RELAYS(void) {
    pinMode(relayCoolPin, OUTPUT);
    pinMode(relayHeatPin, OUTPUT);

    // Initialize all the times to the same to protect
    // everything if there was a power outage.
    ts_heatOFF = Time.now();
    ts_heatON  = ts_heatOFF;
    ts_coolOFF = ts_heatOFF;
    ts_coolON  = ts_heatOFF;  // - (min_cool_time * 2);

    heat_runtime = 0;
    cool_runtime = 0;

    idle();

    heat_cool_Status = IDLE;

    digitalWrite(relayCoolPin, LOW);
    digitalWrite(relayHeatPin, LOW);
}

//----------------------------------------------------------------------------
// Turn on heat if the duty cycle allows it
//----------------------------------------------------------------------------
void RELAYS::heatON(void) {
    time_t now = Time.now();

    if (heat_cool_Status == COOLING_ON) {
        if ((now - ts_coolON) > MIN_COOL_TIME) {
            idle();
        }
    }

    if (heat_cool_Status == IDLE) {
        if (((now - ts_heatOFF) > MIN_HEAT_OFF_ON) &&
            ((now - ts_coolOFF) > MIN_COOL_HEAT)) {
            digitalWrite(relayCoolPin, LOW);
            digitalWrite(relayHeatPin, HIGH);
            heat_cool_Status = HEATER_ON;
            ts_heatON = now;
        } else {
            digitalWrite(relayHeatPin, LOW);  // always turn off, just in case
            digitalWrite(relayCoolPin, LOW);  // since we're in an idle state
        }
    }

    if (heat_cool_Status == HEATER_ON) {
        if ((now - ts_heatON) > MAX_HEAT_TIME) {
            idle();
        } else {  // these should already be set, but just in case
            digitalWrite(relayCoolPin, LOW);
            digitalWrite(relayHeatPin, HIGH);
        }
    }
}

//----------------------------------------------------------------------------
// Turn on cooling if the duty cycle allows it
//----------------------------------------------------------------------------
void RELAYS::coolON(void) {
    time_t now = Time.now();

    if (heat_cool_Status == HEATER_ON) {
        idle();
    }

    if (heat_cool_Status == IDLE) {
        if (((now - ts_coolOFF) > min_cool_off_on) &&
            ((now - ts_heatOFF) > MIN_HEAT_COOL)) {
                digitalWrite(relayHeatPin, LOW);
                digitalWrite(relayCoolPin, HIGH);
                heat_cool_Status = COOLING_ON;
                ts_coolON = now;
        } else {
            digitalWrite(relayHeatPin, LOW);  // always turn off, just in case
            digitalWrite(relayCoolPin, LOW);  // since we're in an idle state
        }
    }

    if (heat_cool_Status == COOLING_ON) {
        if ((now - ts_coolON) > MAX_COOL_TIME) {
            idle();
        }
    }

    if (heat_cool_Status == COOLING_ON) { // should be already set, but what the heck
        digitalWrite(relayHeatPin, LOW);
        digitalWrite(relayCoolPin, HIGH);
    }
}

//----------------------------------------------------------------------------
// Turn off heating and cooling if duty cycle allows it
//----------------------------------------------------------------------------
void RELAYS::idle(void) {
    time_t now = Time.now();

    if (heat_cool_Status == HEATER_ON) {
        if ((now - ts_heatON) > MIN_HEAT_TIME) {
            ts_heatOFF = now;
            heat_cool_Status = IDLE;
            heat_runtime += ts_heatOFF - ts_heatON;
        }
    }

    if (heat_cool_Status == COOLING_ON) {
        if ((now - ts_coolON) > MIN_COOL_TIME) {
            ts_coolOFF = now;
            heat_cool_Status = IDLE;
            cool_runtime += ts_coolOFF - ts_coolON;
        }
    }

    if (heat_cool_Status == IDLE) {
        digitalWrite(relayHeatPin, LOW);  // always turn off, just in case
        digitalWrite(relayCoolPin, LOW);  // since heat or cool might have been on
    }
}

//----------------------------------------------------------------------------
// Expose the current status
//----------------------------------------------------------------------------
mode_heat_cool RELAYS::getHeatCoolStatus() {
    return heat_cool_Status;
}

//----------------------------------------------------------------------------
// Expose the how long the heater has been running
//----------------------------------------------------------------------------
unsigned int RELAYS::get_heat_runtime() { // number of seconds heat has run
    if (heat_cool_Status == HEATER_ON) {
        return heat_runtime + (Time.now() - ts_heatON);
    }

    return heat_runtime; // cooling on or idle
}

//----------------------------------------------------------------------------
// Expose the how long cooling has been running
//----------------------------------------------------------------------------
unsigned int RELAYS::get_cool_runtime() { // number of seconds cooling has run
    if (heat_cool_Status == COOLING_ON) {
        return cool_runtime + (Time.now() - ts_coolON);
    }

    return cool_runtime; // heat on or idle
}
