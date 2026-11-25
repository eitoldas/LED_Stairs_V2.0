#pragma once

#include <Arduino.h>
#include <Preferences.h>

/*
 * Settings that can be changed at runtime and survive a reboot. They are kept
 * in the ESP32's NVS flash, one key per value; anything not saved yet falls
 * back to its default from config.h / animation_config.h / network_config.h.
 */

struct StairSettings
{
    uint16_t stepDelayMs;
    uint16_t holdDelayMs;

    uint32_t retriggerCooldownMs;
    uint32_t exitOffsetMs;

    bool    automaticDaylight;
    uint8_t nightStartHour;
    uint8_t nightStartMinute;
    uint8_t nightEndHour;
    uint8_t nightEndMinute;

    // Fixed network address; a zero ip means DHCP and the rest are ignored
    uint32_t staticIp;
    uint32_t gateway;
    uint32_t subnet;
    uint32_t dns;
};

class SettingsStore
{
public:
    void init();

    const StairSettings& get() const;

    bool save(const StairSettings& settings);
    void restoreDefaults();

    static StairSettings defaults();
    static bool isValid(const StairSettings& settings);

private:
    Preferences   m_preferences;
    StairSettings m_settings = defaults();
};

extern SettingsStore Settings;
