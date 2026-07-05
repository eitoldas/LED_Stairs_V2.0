#include <WiFi.h>

#include "network_config.h"
#include "wifi_driver.h"

void WiFiDriver::init(const char* ssid, const char* password)
{
    m_ssid     = ssid;
    m_password = password;

    WiFi.mode(WIFI_STA);
    WiFi.begin(m_ssid, m_password);
    m_lastAttemptMs = millis();
}

void WiFiDriver::update()
{
    if (isConnected())
    {
        return;
    }

    uint32_t now = millis();
    if (now - m_lastAttemptMs < WIFI_RECONNECT_INTERVAL_MS)
    {
        return;
    }

    m_lastAttemptMs = now;
    WiFi.disconnect();
    WiFi.begin(m_ssid, m_password);
}

bool WiFiDriver::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}
