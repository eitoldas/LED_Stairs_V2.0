#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#pragma once

#include <Arduino.h>

#if __has_include("secrets.h")
    #include "secrets.h"
#else
    #error "Copy include/secrets.example.h to include/secrets.h and fill in your WiFi credentials."
#endif

/*
 * NTP / timezone (UK with automatic BST)
 */

constexpr char NTP_SERVER[] = "pool.ntp.org";
constexpr char TIMEZONE[]   = "GMT0BST,M3.5.0/1,M10.5.0";

constexpr uint32_t NTP_SYNC_TIMEOUT_MS = 10000UL;

/*
 * WiFi reconnect
 */

constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 30000UL;

/*
 * Over-the-air updates. The board answers on <OTA_HOSTNAME>.local, and an
 * upload is only accepted with OTA_PASSWORD from secrets.h.
 */

constexpr char OTA_HOSTNAME[] = "led-stairs";

/*
 * Daily refresh. Once a day at this local time, fetch fresh NTP time and then
 * the day's sunrise and sunset. It also runs once as soon as WiFi first connects.
 * A failed refresh is retried after the retry interval.
 */

constexpr uint8_t DAILY_REFRESH_HOUR   = 3;
constexpr uint8_t DAILY_REFRESH_MINUTE = 0;

constexpr uint32_t REFRESH_RETRY_INTERVAL_MS = 5UL * 60UL * 1000UL;

/*
 * Automatic daylight. When on, lights are allowed from sunset to sunrise as
 * reported by api.sunrisesunset.io for this location. The API answers in the
 * location's own local time, so TIMEZONE above must match the location.
 */

constexpr bool AUTOMATIC_DAYLIGHT = true;

constexpr double SUN_LATITUDE  = 52.112349;
constexpr double SUN_LONGITUDE = -0.419395;

constexpr char     SUN_API_URL[]      = "https://api.sunrisesunset.io/json";
constexpr uint32_t SUN_API_TIMEOUT_MS = 5000UL;

/*
 * Night window — sensor animations only run inside this period (24 h clock).
 * Crosses midnight when NIGHT_START > NIGHT_END (e.g. 18:00 → 07:00).
 * Used when AUTOMATIC_DAYLIGHT is off, or until the first sun lookup succeeds.
 */

constexpr uint8_t NIGHT_START_HOUR   = 20;
constexpr uint8_t NIGHT_START_MINUTE = 0;
constexpr uint8_t NIGHT_END_HOUR     = 6;
constexpr uint8_t NIGHT_END_MINUTE   = 0;

#endif
