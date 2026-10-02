#include <WiFi.h>
#include <esp_sntp.h>
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

    // SNTP only re-polls every few hours on its own, so ask for a fresh
    // answer now and wait for it to arrive.
    sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
    sntp_restart();

    uint32_t start = millis();
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
    {
        if (millis() - start >= NTP_SYNC_TIMEOUT_MS)
        {
            return false;
        }

        delay(10);
    }

    struct tm timeinfo {};
    if (!::getLocalTime(&timeinfo, 0) || timeinfo.tm_year < (2020 - 1900))
    {
        return false;
    }

    m_synced = true;
    return true;
}

bool NTPDriver::isSynced() const
{
    return m_synced;
}

bool NTPDriver::getLocalTime(struct tm* timeinfo) const
{
    return ::getLocalTime(timeinfo);
}
