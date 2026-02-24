#pragma once

#include <Arduino.h>
#include "LEDDriver.h"

class LEDModule
{
public:
    enum Animation
    {
        ANIM_NONE,
        ANIM_CHASE,
        ANIM_WAVE,
    };

    LEDModule(LEDDriver &driver);

    // Call from setup to initialize module
    void begin();

    // Non-blocking tick: call from loop()
    void tick();

    // Start a particular animation with parameters:
    // speed_ms - how long between animation steps (smaller = faster)
    // min_brightness / max_brightness (0-255)
    void startAnimation(Animation a, uint32_t speed_ms = 50, uint8_t min_b = 0, uint8_t max_b = 255);

    void stopAnimation();

    // convenience: trigger from sensor - queues the animation for some duration
    void triggerSensor(Animation a, uint32_t duration_ms = 3000);

private:
    LEDDriver &_drv;

    // animation state
    Animation _anim;
    uint32_t _speed_ms;
    uint8_t _min_b, _max_b;
    uint32_t _last_step_ms;
    uint32_t _anim_end_ms;

    // internal animation positions
    int _pos;
    float _phase;
};
