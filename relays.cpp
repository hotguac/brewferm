/*

This module holds the logic to drive the cooling and heating of the
fermentation chamber.

*/

#include "relays.h"
#include "application.h"

RELAYS::RELAYS(void) {
  pinMode(relayCoolPin, OUTPUT);
  pinMode(relayHeatPin, OUTPUT);

  heatOFF();
  coolOFF();
}

void RELAYS::heatON(void) {
  digitalWrite(relayHeatPin, HIGH);
  heatStatus = ON;
}

void RELAYS::heatOFF(void) {
  digitalWrite(relayHeatPin, LOW);
  heatStatus = OFF;
}

void RELAYS::coolON(void) {
  digitalWrite(relayCoolPin, ON);
  coolStatus = ON;
}

void RELAYS::coolOFF(void) {
  digitalWrite(relayCoolPin, LOW);
  coolStatus = OFF;
}

RELAYS::mode_t RELAYS::getHeatStatus() {
  return heatStatus;
}

RELAYS::mode_t RELAYS::getCoolStatus() {
  return coolStatus;
}
