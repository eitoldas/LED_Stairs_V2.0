#include "LEDModule.h"

LEDModule::LEDModule(LEDDriver &driver)
    : _drv(driver), _anim(ANIM_NONE), _speed_ms(50),
      _min_b(0), _max_b(255), _last_step_ms(0),
      _anim_end_ms(0), _pos(0), _phase(0.0f) {}

void LEDModule::begin()
{
    // nothing to do for now
}

void LEDModule::startAnimation(Animation a, uint32_t speed_ms, uint8_t min_b, uint8_t max_b)
{
    _anim = a;
    _speed_ms = speed_ms;
    _min_b = min_b;
    _max_b = max_b;
    _last_step_ms = millis();
    _pos = 0;
    _phase = 0.0f;
    _anim_end_ms = 0; // run until stopped
}

void LEDModule::stopAnimation()
{
    _anim = ANIM_NONE;
    _drv.setAll(0);
}

void LEDModule::triggerSensor(Animation a, uint32_t duration_ms)
{
    startAnimation(a, _speed_ms, _min_b, _max_b);
    _anim_end_ms = millis() + duration_ms;
}

void LEDModule::tick()
{
    if (_anim == ANIM_NONE)
        return;

    uint32_t now = millis();
    if (_anim_end_ms != 0 && now >= _anim_end_ms)
    {
        stopAnimation();
        return;
    }

    if (now - _last_step_ms < _speed_ms)
        return;
    _last_step_ms = now;

    size_t N = _drv.size();
    if (N == 0)
        return;

    switch (_anim)
    {
    case ANIM_CHASE:
    {
        // single bright LED moving across with fade tails
        for (size_t i = 0; i < N; ++i)
        {
            int dist = abs((int)i - _pos);
            int b = 0;
            if (dist == 0)
                b = _max_b;
            else if (dist < 4)
            {
                // simple falloff
                float t = 1.0f - (dist / 4.0f);
                b = _min_b + (int)((_max_b - _min_b) * t);
            }
            else
            {
                b = _min_b;
            }
            _drv.setBrightness(i, (uint8_t)b);
        }
        _pos++;
        if (_pos >= (int)N)
            _pos = 0;
        break;
    }

    case ANIM_WAVE:
    {
        // sine wave across LEDs
        _phase += 0.3f; // step
        for (size_t i = 0; i < N; ++i)
        {
            float x = _phase + (i * 0.6f);
            float s = (sin(x) + 1.0f) * 0.5f; // 0..1
            uint8_t b = _min_b + (uint8_t)((_max_b - _min_b) * s);
            _drv.setBrightness(i, b);
        }
        break;
    }

    default:
        break;
    }
}
