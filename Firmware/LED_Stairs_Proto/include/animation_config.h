#ifndef ANIMATION_CONFIG_H
#define ANIMATION_CONFIG_H

#pragma once

#include <Arduino.h>

/*
 * Run-up / run-down animation
 */

constexpr uint16_t ANIM_STEP_DELAY_MS   = 300;
constexpr uint16_t STAIR_HOLD_DELAY_MS  = 2000;

/*
 * Blink animation
 */

constexpr uint16_t BLINK_INTERVAL_MS    = 200;
constexpr uint8_t  BLINK_COUNT          = 2;

/*
 * Startup animation
 */

constexpr uint16_t STARTUP_STEP_DELAY_MS        = 100;
constexpr uint16_t STARTUP_HOLD_MS              = 2000;
constexpr uint16_t STARTUP_BLINK_INTERVAL_MS    = 200;
constexpr uint8_t  STARTUP_BLINK_COUNT          = 3;

#endif
