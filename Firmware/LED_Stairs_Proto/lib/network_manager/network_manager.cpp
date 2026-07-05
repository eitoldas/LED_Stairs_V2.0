#include <time.h>

#include "network_config.h"
#include "network_manager.h"

void NetworkManager::init()
{
    m_wifiDriver.init(WIFI_SSID, WIFI_PASSWORD);
    m_ntpDriver.init(NTP_SERVER, TIMEZONE);
}

void NetworkManager::update()
{
    m_wifiDriver.update();

    if (m_wifiDriver.isConnected())
    {
        m_ntpDriver.update(NTP_RESYNC_INTERVAL_MS);
    }
}

bool NetworkManager::isTimeSynced() const
{
    return m_ntpDriver.isSynced();
}

bool NetworkManager::isWithinNightWindow(const struct tm& timeinfo) const
{
    uint16_t nowMinutes    = (uint16_t)(timeinfo.tm_hour * 60 + timeinfo.tm_min);
    uint16_t startMinutes  = (uint16_t)(NIGHT_START_HOUR * 60 + NIGHT_START_MINUTE);
    uint16_t endMinutes    = (uint16_t)(NIGHT_END_HOUR * 60 + NIGHT_END_MINUTE);

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
