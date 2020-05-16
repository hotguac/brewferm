#include "indicators.h"

// public functions
CustomStatus customStatus(LED_PRIORITY_IMPORTANT);

INDICATORS::INDICATORS(void) {

}

void INDICATORS::init(void) {
    // Activate custom status
    customStatus.setActive(true);
}

void setStatus(float) {

}

// private functions
void INDICATORS::setInit(void) {

}


void INDICATORS::setPaused(void) {

}

void INDICATORS::setInTemp(void) {

}

void INDICATORS::setNearTemp(void) {

}

void INDICATORS::setOutOfRange(float offset) {

}

void INDICATORS::setStatus(float offset) {

}
