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
  Serial.printf("Heat ON\n\r");
}

void RELAYS::heatOFF(void) {
  digitalWrite(relayHeatPin, LOW);
  heatStatus = OFF;
  Serial.printf("Heat OFF\n\r");
}

void RELAYS::coolON(void) {
  digitalWrite(relayCoolPin, ON);
  coolStatus = ON;
  Serial.printf("Cool ON\n\r");
}

void RELAYS::coolOFF(void) {
  digitalWrite(relayCoolPin, LOW);
  coolStatus = OFF;
  Serial.printf("Cool OFF\n\r");
}

RELAYS::mode_t RELAYS::getHeatStatus() {
  return heatStatus;
}

RELAYS::mode_t RELAYS::getCoolStatus() {
  return coolStatus;
}
