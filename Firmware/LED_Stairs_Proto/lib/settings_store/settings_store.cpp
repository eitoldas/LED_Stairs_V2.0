#include <Arduino.h>

#include "animation_config.h"
#include "config.h"
#include "logger.h"
#include "network_config.h"
#include "settings_store.h"

SettingsStore Settings;

static const char NAMESPACE[] = "settings";

void SettingsStore::init()
{
    m_preferences.begin(NAMESPACE, false);

    StairSettings fallback = defaults();
    StairSettings loaded;

    loaded.stepDelayMs         = m_preferences.getUShort("stepMs",       fallback.stepDelayMs);
    loaded.holdDelayMs         = m_preferences.getUShort("holdMs",       fallback.holdDelayMs);
    loaded.retriggerCooldownMs = m_preferences.getULong("cooldownMs",    fallback.retriggerCooldownMs);
    loaded.exitOffsetMs        = m_preferences.getULong("exitOffsetMs",  fallback.exitOffsetMs);
    loaded.automaticDaylight   = m_preferences.getBool("autoDaylight",   fallback.automaticDaylight);
    loaded.nightStartHour      = m_preferences.getUChar("nightStartH",   fallback.nightStartHour);
    loaded.nightStartMinute    = m_preferences.getUChar("nightStartM",   fallback.nightStartMinute);
    loaded.nightEndHour        = m_preferences.getUChar("nightEndH",     fallback.nightEndHour);
    loaded.nightEndMinute      = m_preferences.getUChar("nightEndM",     fallback.nightEndMinute);
    loaded.staticIp            = m_preferences.getULong("staticIp",      fallback.staticIp);
    loaded.gateway             = m_preferences.getULong("gateway",       fallback.gateway);
    loaded.subnet              = m_preferences.getULong("subnet",        fallback.subnet);
    loaded.dns                 = m_preferences.getULong("dns",           fallback.dns);

    if (isValid(loaded))
    {
        m_settings = loaded;
        Log.println("Settings: loaded");
    }
    else
    {
        m_settings = fallback;
        Log.println("Settings: saved values invalid, using defaults");
    }
}

const StairSettings& SettingsStore::get() const
{
    return m_settings;
}

bool SettingsStore::save(const StairSettings& settings)
{
    if (!isValid(settings))
    {
        return false;
    }

    m_preferences.putUShort("stepMs",      settings.stepDelayMs);
    m_preferences.putUShort("holdMs",      settings.holdDelayMs);
    m_preferences.putULong("cooldownMs",   settings.retriggerCooldownMs);
    m_preferences.putULong("exitOffsetMs", settings.exitOffsetMs);
    m_preferences.putBool("autoDaylight",  settings.automaticDaylight);
    m_preferences.putUChar("nightStartH",  settings.nightStartHour);
    m_preferences.putUChar("nightStartM",  settings.nightStartMinute);
    m_preferences.putUChar("nightEndH",    settings.nightEndHour);
    m_preferences.putUChar("nightEndM",    settings.nightEndMinute);
    m_preferences.putULong("staticIp",     settings.staticIp);
    m_preferences.putULong("gateway",      settings.gateway);
    m_preferences.putULong("subnet",       settings.subnet);
    m_preferences.putULong("dns",          settings.dns);

    m_settings = settings;
    Log.println("Settings: saved");
    return true;
}

void SettingsStore::restoreDefaults()
{
    m_preferences.clear();
    m_settings = defaults();
    Log.println("Settings: restored defaults");
}

StairSettings SettingsStore::defaults()
{
    StairSettings settings;

    settings.stepDelayMs         = ANIM_STEP_DELAY_MS;
    settings.holdDelayMs         = STAIR_HOLD_DELAY_MS;
    settings.retriggerCooldownMs = RETRIGGER_COOLDOWN_MS;
    settings.exitOffsetMs        = EXIT_OFFSET_MS;
    settings.automaticDaylight   = AUTOMATIC_DAYLIGHT;
    settings.nightStartHour      = NIGHT_START_HOUR;
    settings.nightStartMinute    = NIGHT_START_MINUTE;
    settings.nightEndHour        = NIGHT_END_HOUR;
    settings.nightEndMinute      = NIGHT_END_MINUTE;
    settings.staticIp            = 0;
    settings.gateway             = 0;
    settings.subnet              = 0;
    settings.dns                 = 0;

    return settings;
}

bool SettingsStore::isValid(const StairSettings& settings)
{
    return settings.stepDelayMs >= 50 && settings.stepDelayMs <= 5000
        && settings.holdDelayMs <= 60000
        && settings.retriggerCooldownMs <= 60000
        && settings.exitOffsetMs <= 30000
        && settings.nightStartHour < 24 && settings.nightStartMinute < 60
        && settings.nightEndHour < 24   && settings.nightEndMinute < 60
        && (settings.staticIp == 0 || (settings.gateway != 0 && settings.subnet != 0));
}
