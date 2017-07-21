/*

This module holds the logic to drive the cooling and heating of the
fermentation chamber.

*/

#include "relays.h"
#include "application.h"

RELAYS::RELAYS(void) {
  pinMode(relayCoolPin, OUTPUT);
  pinMode(relayHeatPin, OUTPUT);

  // Initialize all the times to the same to protect
  // everything if there was a power outage.
  ts_heatOFF = Time.now();
  ts_heatON  = ts_heatOFF;
  ts_coolOFF = ts_heatOFF;
  ts_coolON  = ts_heatOFF - (min_cool_time * 2);

  heatOFF();
  coolOFF();
  digitalWrite(relayCoolPin, OFF);
  digitalWrite(relayHeatPin, LOW);
}

void RELAYS::heatON(void) {
  time_t now = Time.now();

  if (heatStatus != ON) {
    if (((now - ts_heatOFF) > min_heat_off_on) &&
        ((now - ts_coolOFF) > min_cool_heat) &&
        ((now - ts_coolON) > min_cool_time)) {
      digitalWrite(relayHeatPin, HIGH);
      heatStatus = ON;
      ts_heatON = now;
    } else {
      heatStatus = PENDING;
    }
  }
}

void RELAYS::coolON(void) {
  time_t now = Time.now();

  if (coolStatus != ON) {
    if (((now - ts_coolOFF) > min_cool_off_on) &&
        ((now - ts_heatOFF) > min_heat_cool)) {
      digitalWrite(relayCoolPin, ON);
      coolStatus = ON;
      ts_coolON = now;
    } else {
      coolStatus = PENDING;
    }
  }
}

void RELAYS::heatOFF(void) {
  digitalWrite(relayHeatPin, LOW); // always turn off, just in case
  if (heatStatus == ON) {
    ts_heatOFF = Time.now(); // only set ts if it WAS on
  }
  heatStatus = OFF;
}

void RELAYS::coolOFF(void) {
  time_t now = Time.now();

  if (coolStatus == ON) {
    if ((now - ts_coolON) > min_cool_time) {
      ts_coolOFF = now;
      digitalWrite(relayCoolPin, LOW);
      coolStatus = OFF;
    }
  }
}

RELAYS::mode_t RELAYS::getHeatStatus() {
  return heatStatus;
}

RELAYS::mode_t RELAYS::getCoolStatus() {
  return coolStatus;
}

void RELAYS::controlTemp(double current, double target) {
  // If the chamber is more than half a degree warmer than
  // the PID output target then turn on cooling, if more than
  // half degree cool turn on heat

  if (current - target > 0.5) {
    coolON();
    heatOFF();
  } else if (current - target < -0.5) {
    heatON();
    coolOFF();
  } else {
    heatOFF();
    coolOFF();
  }

}  // RELAYS::controlTemp
