#include <WiFi.h>
#include <time.h>

#include "network_config.h"
#include "ntp_driver.h"

void NTPDriver::init(const char* server, const char* timezone)
{
    m_server   = server;
    m_timezone = timezone;

    configTzTime(m_timezone, m_server);
}

bool NTPDriver::sync()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    struct tm timeinfo {};
    if (!::getLocalTime(&timeinfo, NTP_SYNC_TIMEOUT_MS))
    {
        return false;
    }

    if (timeinfo.tm_year < (2020 - 1900))
    {
        return false;
    }

    m_synced     = true;
    m_lastSyncMs = millis();
    return true;
}

void NTPDriver::update(uint32_t resyncIntervalMs)
{
    if (!m_synced)
    {
        sync();
        return;
    }

    if (millis() - m_lastSyncMs >= resyncIntervalMs)
    {
        sync();
    }
}

bool NTPDriver::isSynced() const
{
    return m_synced;
}

bool NTPDriver::getLocalTime(struct tm* timeinfo) const
{
    return ::getLocalTime(timeinfo);
}
