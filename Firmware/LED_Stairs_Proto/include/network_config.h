#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#pragma once

#include <Arduino.h>

/*
 * WiFi credentials — update before flashing
 */

constexpr char WIFI_SSID[]     = "REDACTED Mesh";
constexpr char WIFI_PASSWORD[] = "REDACTED";

/*
 * NTP / timezone (UK with automatic BST)
 */

constexpr char NTP_SERVER[] = "pool.ntp.org";
constexpr char TIMEZONE[]   = "GMT0BST,M3.5.0/1,M10.5.0";

constexpr uint32_t NTP_RESYNC_INTERVAL_MS = 24UL * 60UL * 60UL * 1000UL;
constexpr uint32_t NTP_SYNC_TIMEOUT_MS    = 10000UL;

/*
 * WiFi reconnect
 */

constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 30000UL;

/*
 * Night window — sensor animations only run inside this period (24 h clock).
 * Crosses midnight when NIGHT_START > NIGHT_END (e.g. 18:00 → 07:00).
 */

constexpr uint8_t NIGHT_START_HOUR   = 20;
constexpr uint8_t NIGHT_START_MINUTE = 0;
constexpr uint8_t NIGHT_END_HOUR     = 6;
constexpr uint8_t NIGHT_END_MINUTE   = 0;

#endif
