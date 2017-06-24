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
  ts_coolON  = ts_heatOFF;

  heatOFF();
  coolOFF();
}

void RELAYS::heatON(void) {
  time_t now = Time.now();
  if ((difftime(now, ts_heatOFF) > min_heat_off_on) &&
      (difftime(now, ts_coolOFF) > min_cool_heat)) {
        digitalWrite(relayHeatPin, HIGH);
        heatStatus = ON;
        ts_heatON = now;
        Serial.println("Heat ON");
      } else {
        Serial.println("Pending Heat ON");
      }
}

void RELAYS::heatOFF(void) {
  digitalWrite(relayHeatPin, LOW);
  heatStatus = OFF;
  ts_heatOFF = Time.now();
  Serial.println("Heat OFF");
}

void RELAYS::coolON(void) {
  time_t now = Time.now();
  if ((difftime(now, ts_heatOFF) > min_heat_cool) &&
      (difftime(now, ts_coolOFF) > min_cool_off_on)) {
        digitalWrite(relayCoolPin, ON);
        coolStatus = ON;
        ts_coolON = now;
        Serial.println("Cool ON");
      } else {
        Serial.println("Pending Cool ON");
      }
}

void RELAYS::coolOFF(void) {
  digitalWrite(relayCoolPin, LOW);
  coolStatus = OFF;
  ts_coolOFF = Time.now();
  Serial.println("Cool OFF");
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
  } else if (current - target < 0.5) {
    heatON();
    coolOFF();
  } else {
    heatOFF();
    coolOFF();
  }

}  // RELAYS::controlTemp
