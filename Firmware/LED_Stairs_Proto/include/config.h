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

constexpr uint32_t RETRIGGER_COOLDOWN_MS = 2000;

/*
 * Far sensor exit window. A walker trips the opposite sensor as the light sweep
 * finishes, so a trigger this far either side of that moment is taken as the
 * same walker leaving. Anything outside the window is treated as someone new.
 */

constexpr uint32_t EXIT_OFFSET_MS = 4000;

/*
 * Serial output
 */

constexpr uint32_t SERIAL_BAUD = 115200;

/*
 * Current sensor (CH70120CB3PR: hall effect, 3.3 V supply, bidirectional +/-20 A,
 * 66 mV per amp around a fixed 1.65 V zero-current output).
 *
 * VIOUT must land on an ADC1 pin. GPIO21 has no ADC on the ESP32-S3, and ADC2
 * pins cannot be read while WiFi is active. GPIO2 and GPIO8 are the free ones.
 */

constexpr uint8_t CURRENT_SENSE_PIN = 2;

constexpr uint16_t CURRENT_SENSITIVITY_MV_PER_A = 66;
constexpr uint16_t CURRENT_ZERO_MV              = 1650;

constexpr uint8_t  CURRENT_SAMPLE_COUNT       = 16;
constexpr uint32_t CURRENT_SAMPLE_INTERVAL_MS = 100;
constexpr uint32_t CURRENT_REPORT_INTERVAL_MS = 500;

/*
 * Supply voltage feeding the LED strips. Only used to turn amps into watts.
 */

constexpr float LOAD_VOLTAGE = 12.0f;

#endif