#ifndef INDICATORS_H_
#define INDICATORS_H_

#include "Particle.h"
#include "rgbled.h"

#define IND_COLOR_PREAMBLE1 0x000e0e0e
#define IND_COLOR_PREAMBLE2 0x008f8f8f
#define IND_COLOR_INIT      RGB_COLOR_BLUE
#define IND_COLOR_PAUSED    RGB_COLOR_BLUE
#define IND_COLOR_IN_TEMP   RGB_COLOR_GREEN
#define IND_COLOR_NEAR_TEMP RGB_COLOR_YELLOW
#define IND_COLOR_OUT1      RGB_COLOR_ORANGE
#define IND_COLOR_OUT2      RGB_COLOR_RED

#define MAX_COLOR_SEQ_SIZE 10

class INDICATORS {
    public:
        //
        INDICATORS(void);

        void init(void);

        void setPaused(void);
        void setInTemp(void);
        void setNearTemp(void);
        void setOutOfRange(float);
        void setStatus(float);
};

class CustomStatus: public LEDStatus {
public:
    explicit CustomStatus(LEDPriority priority) :
        LEDStatus(LED_PATTERN_CUSTOM, priority),
        colorIndex(0),
        colorTicks(0) {
    }

    void setColors(uint32_t color_seq[], size_t count);

protected:
    virtual void update(system_tick_t ticks) override {
        // Change status color every 300 milliseconds
        colorTicks += ticks;
        if (colorTicks > 300) {
            if (++colorIndex == colorCount) {
                colorIndex = 0;
            }
            setColor(colors[colorIndex]);
            colorTicks = 0;
        }
    }

private:
    size_t colorIndex;
    system_tick_t colorTicks;

    uint32_t colors[MAX_COLOR_SEQ_SIZE];
    size_t colorCount;
};

#endif  // INDICATORS_H_
