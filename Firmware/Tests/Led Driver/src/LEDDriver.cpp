#include "LEDDriver.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

// Constructor
LEDDriver::LEDDriver(const uint8_t *pins, size_t count)
    : _pins(pins), _count(count), _brightness(nullptr),
      _plane(0), _timer(nullptr), _running(false)
{
    _lock = portMUX_INITIALIZER_UNLOCKED;
    _brightness = (uint8_t *)malloc(_count);
    if (_brightness)
    {
        memset(_brightness, 0, _count);
    }
}

bool LEDDriver::begin(uint32_t frame_us)
{
    if (!_brightness)
        return false;

    // compute unit microseconds: frame_us / 255
    if (frame_us < 255)
        frame_us = 255; // minimum safe
    _unit_us = frame_us / 255u;

    // configure pins as outputs and set LOW initially
    for (size_t i = 0; i < _count; ++i)
    {
        pinMode(_pins[i], OUTPUT);
        gpio_set_level((gpio_num_t)_pins[i], 0);
    }

    // create esp_timer (one-shot). We'll re-start it with different intervals from the callback
    esp_timer_create_args_t args;
    args.callback = &_timer_cb;
    args.arg = (void *)this;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "leddrv_timer";

    if (esp_timer_create(&args, &_timer) != ESP_OK)
    {
        return false;
    }

    _plane = 0;
    _running = true;

    // Start first callback immediately (0 us)
    esp_timer_start_once(_timer, 0);

    return true;
}

void LEDDriver::stop()
{
    if (_timer)
    {
        esp_timer_stop(_timer);
        esp_timer_delete(_timer);
        _timer = nullptr;
    }
    _running = false;
    // turn off outputs
    for (size_t i = 0; i < _count; ++i)
    {
        gpio_set_level((gpio_num_t)_pins[i], 0);
    }
}

void LEDDriver::setBrightness(size_t idx, uint8_t value)
{
    if (idx >= _count)
        return;
    portENTER_CRITICAL(&_lock);
    _brightness[idx] = value;
    portEXIT_CRITICAL(&_lock);
}

void LEDDriver::setAll(uint8_t value)
{
    portENTER_CRITICAL(&_lock);
    for (size_t i = 0; i < _count; ++i)
        _brightness[i] = value;
    portEXIT_CRITICAL(&_lock);
}

void LEDDriver::_timer_cb(void *arg)
{
    LEDDriver *self = (LEDDriver *)arg;
    if (self)
        self->onTimer();
}

void LEDDriver::onTimer()
{
    if (!_running)
        return;

    // apply current plane
    applyPlane(_plane);

    // duration for this plane (units * (1 << plane))
    uint32_t dur = _unit_us * (1u << _plane);

    // advance plane
    _plane++;
    if (_plane > 7)
        _plane = 0;

    // restart one-shot timer for the next plane
    esp_timer_start_once(_timer, dur);
}

void LEDDriver::applyPlane(uint8_t plane)
{
    portENTER_CRITICAL(&_lock);
    for (size_t i = 0; i < _count; ++i)
    {
        uint8_t b = _brightness[i];
        bool on = (b & (1u << plane)) != 0;
        // MOSFETs are active-high so 'on' -> set HIGH
        gpio_set_level((gpio_num_t)_pins[i], on ? 1 : 0);
    }
    portEXIT_CRITICAL(&_lock);
}
