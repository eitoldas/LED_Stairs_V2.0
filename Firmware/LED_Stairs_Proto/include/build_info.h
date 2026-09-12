#pragma once

#include <Arduino.h>

/*
 * Build identity. Bump these by hand when the board or the firmware changes.
 * BUILD_TIMESTAMP is filled in by the compiler when device_info.cpp is built.
 */

constexpr char HARDWARE_VERSION[] = "v2.0";
constexpr char FIRMWARE_VERSION[] = "v2.0.0";

constexpr char BUILD_TIMESTAMP[] = __DATE__ " " __TIME__;
