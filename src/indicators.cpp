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
    customStatus.setColors(init_colors, init_colorCount);
    customStatus.setActive(true);
}

void INDICATORS::setPaused(void) {
    uint32_t paused_colors[] = {
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2,
        IND_COLOR_PAUSED,
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2
    };

    size_t paused_colorCount =
        sizeof(paused_colors) /
        sizeof(paused_colors[0]);

    customStatus.setColors(paused_colors, paused_colorCount);
}

void INDICATORS::setInTemp(void) {
    uint32_t inTemp_colors[] = {
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2,
        IND_COLOR_IN_TEMP,
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2
    };

    size_t inTemp_colorCount =
        sizeof(inTemp_colors) /
        sizeof(inTemp_colors[0]);

    customStatus.setColors(inTemp_colors, inTemp_colorCount);
}

void INDICATORS::setNearTemp(void) {
    uint32_t nearTemp_colors[] = {
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2,
        IND_COLOR_NEAR_TEMP,
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2
    };

    size_t nearTemp_colorCount =
        sizeof(nearTemp_colors) /
        sizeof(nearTemp_colors[0]);

    customStatus.setColors(nearTemp_colors, nearTemp_colorCount);
}

void INDICATORS::setOutOfRange(float offset) {
    uint32_t out_color;

    if (abs(offset) < 1.5) { //TODO: add define in header
        out_color = IND_COLOR_OUT1;
    } else {
        out_color = IND_COLOR_OUT2;
    }

    uint32_t outOfRange_colors[] = {
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2,
        out_color,
        IND_COLOR_PREAMBLE1,
        IND_COLOR_PREAMBLE2
    };

    size_t outOfRange_colorCount =
        sizeof(outOfRange_colors) /
        sizeof(outOfRange_colors[0]);

    customStatus.setColors(outOfRange_colors, outOfRange_colorCount);
}

void INDICATORS::setStatus(float offset) {
    float a = abs(offset);

    if (a < 0.5) { //TODO: add defines in header
        setInTemp();
    } else if (a < 1.0) {
        setNearTemp();
    } else {
        setOutOfRange(offset);
    }
}
