#ifndef INDICATORS_H_
#define INDICATORS_H_

#include "Particle.h"
#include "rgbled.h"

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

    private:
        void setInit(void);
};

class CustomStatus: public LEDStatus {
public:
    explicit CustomStatus(LEDPriority priority) :
        LEDStatus(LED_PATTERN_CUSTOM, priority),
        colorIndex(0),
        colorTicks(0) {
    }

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

    static const uint32_t colors[];
    static const size_t colorCount;
};

const uint32_t CustomStatus::colors[] = {
    RGB_COLOR_MAGENTA,
    RGB_COLOR_BLUE,
    RGB_COLOR_CYAN,
    RGB_COLOR_GREEN,
    RGB_COLOR_YELLOW
};

const size_t CustomStatus::colorCount =
    sizeof(CustomStatus::colors) /
    sizeof(CustomStatus::colors[0]);


#endif  // INDICATORS_H_
