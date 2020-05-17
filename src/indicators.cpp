#include "indicators.h"

uint32_t init_colors[] = {
    IND_COLOR_PREAMBLE1,
    IND_COLOR_PREAMBLE2,
    IND_COLOR_INIT,
    IND_COLOR_PREAMBLE1,
    IND_COLOR_PREAMBLE2
};

size_t init_colorCount =
    sizeof(init_colors) /
    sizeof(init_colors[0]);

// public functions
CustomStatus customStatus(LED_PRIORITY_IMPORTANT);

void CustomStatus::setColors(uint32_t color_seq[], size_t count) {
    if (count <= MAX_COLOR_SEQ_SIZE) {
        colorCount = init_colorCount;
    } else {
        colorCount = MAX_COLOR_SEQ_SIZE;
    }

    for (size_t i = 0; i < colorCount; ++i) {
        colors[i] = color_seq[i];
    }
}

INDICATORS::INDICATORS(void) {
    customStatus.setColors(init_colors, init_colorCount);
}

void INDICATORS::init(void) {
    // Activate custom status
    customStatus.setActive(true);
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
