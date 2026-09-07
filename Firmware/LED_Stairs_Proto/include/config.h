#ifndef CONFIG_H
#define CONFIG_H

#pragma once

#include <Arduino.h>

/*
 * Stair LED configuration
 */

constexpr uint8_t NUM_LEDS = 14;

constexpr uint8_t LED_PINS[NUM_LEDS] =
{
    1,  4,  5,  6,  7,  15, 16,
    3,  9,  10, 11, 12, 13, 14
};

/*
 * PIR Sensor configuration
 */

constexpr uint8_t SENSOR_A_PIN = 17;
constexpr uint8_t SENSOR_B_PIN = 18;

constexpr uint32_t RETRIGGER_COOLDOWN_MS = 3000;

/*
 * Far sensor exit window. A walker trips the opposite sensor as the light sweep
 * finishes, so a trigger this far either side of that moment is taken as the
 * same walker leaving. Anything outside the window is treated as someone new.
 */

constexpr uint32_t EXIT_OFFSET_MS = 4000;

#endif