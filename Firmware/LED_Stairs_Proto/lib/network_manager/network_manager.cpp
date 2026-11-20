#include <ArduinoOTA.h>
#include <time.h>

#include "logger.h"
#include "network_config.h"
#include "network_manager.h"
#include "settings_store.h"

void NetworkManager::init()
{
    m_wifiDriver.init(WIFI_SSID, WIFI_PASSWORD);
    m_ntpDriver.init(NTP_SERVER, TIMEZONE);
    m_sunTimesDriver.init(SUN_LATITUDE, SUN_LONGITUDE);
}

void NetworkManager::update()
{
    m_wifiDriver.update();

    if (!m_wifiDriver.isConnected())
    {
        return;
    }

    if (!m_otaStarted)
    {
        startOta();
    }

    ArduinoOTA.handle();

    if (isRefreshDue())
    {
        refresh();
    }
}

void NetworkManager::startOta()
{
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() { Log.println("OTA: update starting"); });
    ArduinoOTA.onEnd([]()   { Log.println("OTA: update done, rebooting"); });
    ArduinoOTA.onError([](ota_error_t error) { Log.printf("OTA: update failed (%u)\n", error); });

    ArduinoOTA.begin();
    m_otaStarted = true;
}

bool NetworkManager::isTimeSynced() const
{
    return m_ntpDriver.isSynced();
}

bool NetworkManager::isRefreshDue() const
{
    if (m_hasAttempted && millis() - m_lastAttemptMs < REFRESH_RETRY_INTERVAL_MS)
    {
        return false;
    }

    if (m_lastRefreshDay < 0)
    {
        return true;
    }

    struct tm timeinfo {};
    if (!m_ntpDriver.getLocalTime(&timeinfo))
    {
        return false;
    }

    uint16_t nowMinutes     = (uint16_t)(timeinfo.tm_hour * 60 + timeinfo.tm_min);
    uint16_t refreshMinutes = (uint16_t)(DAILY_REFRESH_HOUR * 60 + DAILY_REFRESH_MINUTE);

    return timeinfo.tm_yday != m_lastRefreshDay && nowMinutes >= refreshMinutes;
}

void NetworkManager::refresh()
{
    m_hasAttempted  = true;
    m_lastAttemptMs = millis();

    if (!m_ntpDriver.sync())
    {
        Log.println("Daily refresh: NTP sync failed, retrying later");
        return;
    }

    if (Settings.get().automaticDaylight && !m_sunTimesDriver.fetch())
    {
        Log.println("Daily refresh: sun times lookup failed, retrying later");
        return;
    }

    struct tm timeinfo {};
    m_ntpDriver.getLocalTime(&timeinfo);
    m_lastRefreshDay = timeinfo.tm_yday;

    if (Settings.get().automaticDaylight)
    {
        uint16_t sunrise = m_sunTimesDriver.sunriseMinutes();
        uint16_t sunset  = m_sunTimesDriver.sunsetMinutes();

        Log.printf("Daily refresh: sunrise %02d:%02d, sunset %02d:%02d\n",
            sunrise / 60, sunrise % 60, sunset / 60, sunset % 60);
    }
    else
    {
        Log.println("Daily refresh: time synced");
    }
}

bool NetworkManager::isWithinNightWindow(const struct tm& timeinfo) const
{
    uint16_t nowMinutes   = (uint16_t)(timeinfo.tm_hour * 60 + timeinfo.tm_min);
    const StairSettings& settings = Settings.get();

    uint16_t startMinutes = (uint16_t)(settings.nightStartHour * 60 + settings.nightStartMinute);
    uint16_t endMinutes   = (uint16_t)(settings.nightEndHour * 60 + settings.nightEndMinute);

    if (settings.automaticDaylight && m_sunTimesDriver.hasTimes())
    {
        startMinutes = m_sunTimesDriver.sunsetMinutes();
        endMinutes   = m_sunTimesDriver.sunriseMinutes();
    }

    if (startMinutes < endMinutes)
    {
        return nowMinutes >= startMinutes && nowMinutes < endMinutes;
    }

    return nowMinutes >= startMinutes || nowMinutes < endMinutes;
}

bool NetworkManager::isLightsAllowed() const
{
    if (IGNORE_NIGHT_WINDOW)
    {
        return true;
    }

    if (!m_ntpDriver.isSynced())
    {
        return true;
    }

    struct tm timeinfo {};
    if (!m_ntpDriver.getLocalTime(&timeinfo))
    {
        return true;
    }

    return isWithinNightWindow(timeinfo);
}
