#pragma once

#include <Arduino.h>

class LEDDriver
{
public:
    // pins[]: array of gpio pin numbers, count: number of leds (e.g. 14)
    LEDDriver(const uint8_t *pins, size_t count);

    // Initialize driver and start PWM. frame_us = total PWM frame time in microseconds (sum of binary weights = 255 units)
    bool begin(uint32_t frame_us = 5000);

    // Stop PWM
    void stop();

    // Set brightness (0-255) for one LED
    void setBrightness(size_t idx, uint8_t value);

    // Set brightness for all LEDs
    void setAll(uint8_t value);

    // Get number of LEDs
    size_t size() const { return _count; }

private:
    const uint8_t *_pins;
    size_t _count;

    // brightness buffer (owned by driver)
    uint8_t *_brightness;

    // current bit-plane (0..7)
    uint8_t _plane;

    // microsecond unit time for single weight (1 unit). durations are unit * (1<<plane)
    uint32_t _unit_us;

    // esp_timer handle
    esp_timer_handle_t _timer;

    // running flag
    volatile bool _running;

    // concurrency lock
    portMUX_TYPE _lock;

    // Timer callback wrapper (static)
    static void _timer_cb(void *arg);

    // instance method called by timer cb
    void onTimer();

    // apply output for current plane
    void applyPlane(uint8_t plane);
};
