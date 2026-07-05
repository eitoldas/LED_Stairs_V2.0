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

constexpr uint32_t RETRIGGER_COOLDOWN_MS = 5000;

constexpr uint16_t LED_STEP_DELAY_MS = 300;

#endif