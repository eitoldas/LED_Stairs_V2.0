#include <time.h>

#include "network_config.h"
#include "network_manager.h"

void NetworkManager::init()
{
    m_wifiDriver.init(WIFI_SSID, WIFI_PASSWORD);
    m_ntpDriver.init(NTP_SERVER, TIMEZONE);
    m_sunTimesDriver.init(SUN_LATITUDE, SUN_LONGITUDE);
}

void NetworkManager::update()
{
    m_wifiDriver.update();

    if (m_wifiDriver.isConnected() && isRefreshDue())
    {
        refresh();
    }
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
        Serial.println("Daily refresh: NTP sync failed, retrying later");
        return;
    }

    if (AUTOMATIC_DAYLIGHT && !m_sunTimesDriver.fetch())
    {
        Serial.println("Daily refresh: sun times lookup failed, retrying later");
        return;
    }

    struct tm timeinfo {};
    m_ntpDriver.getLocalTime(&timeinfo);
    m_lastRefreshDay = timeinfo.tm_yday;

    if (AUTOMATIC_DAYLIGHT)
    {
        uint16_t sunrise = m_sunTimesDriver.sunriseMinutes();
        uint16_t sunset  = m_sunTimesDriver.sunsetMinutes();

        Serial.printf("Daily refresh: sunrise %02d:%02d, sunset %02d:%02d\n",
            sunrise / 60, sunrise % 60, sunset / 60, sunset % 60);
    }
    else
    {
        Serial.println("Daily refresh: time synced");
    }
}

bool NetworkManager::isWithinNightWindow(const struct tm& timeinfo) const
{
    uint16_t nowMinutes   = (uint16_t)(timeinfo.tm_hour * 60 + timeinfo.tm_min);
    uint16_t startMinutes = (uint16_t)(NIGHT_START_HOUR * 60 + NIGHT_START_MINUTE);
    uint16_t endMinutes   = (uint16_t)(NIGHT_END_HOUR * 60 + NIGHT_END_MINUTE);

    if (AUTOMATIC_DAYLIGHT && m_sunTimesDriver.hasTimes())
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
