#include <WiFi.h>

#include "logger.h"
#include "network_config.h"
#include "wifi_driver.h"

static String durationText(uint32_t ms)
{
    char text[16];
    if (ms < 60000UL)
    {
        snprintf(text, sizeof(text), "%lu.%lu s", (unsigned long)(ms / 1000), (unsigned long)(ms % 1000 / 100));
    }
    else
    {
        snprintf(text, sizeof(text), "%lum %lus", (unsigned long)(ms / 60000), (unsigned long)(ms % 60000 / 1000));
    }
    return String(text);
}

// The WiFi library has no name for some codes the router can send
static const char* reasonText(uint8_t reason)
{
    const char* name = WiFi.disconnectReasonName((wifi_err_reason_t)reason);
    return (name != nullptr && name[0] != '\0') ? name : "UNKNOWN";
}

void WiFiDriver::init(const char* ssid, const char* password)
{
    m_ssid     = ssid;
    m_password = password;

    // Only note the reason here; the logging happens in update() on the main loop
    WiFi.onEvent([this](arduino_event_id_t, arduino_event_info_t info)
    {
        m_eventReason  = info.wifi_sta_disconnected.reason;
        m_eventPending = true;
    }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.mode(WIFI_STA);
    WiFi.begin(m_ssid, m_password);

    m_lastAttemptMs  = millis();
    m_offlineSinceMs = m_lastAttemptMs;
}

void WiFiDriver::update()
{
    if (m_eventPending)
    {
        m_eventPending = false;
        handleDisconnect(m_eventReason);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if (!m_online)
        {
            handleConnect();
        }
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
    return m_online;
}

void WiFiDriver::handleDisconnect(uint8_t reason)
{
    // Our own WiFi.disconnect() before a retry, not a real drop
    if (reason == WIFI_REASON_ASSOC_LEAVE)
    {
        return;
    }

    if (m_online)
    {
        m_online         = false;
        m_offlineSinceMs = millis();
        m_failedAttempts = 0;
        m_loggedReason   = reason;

        Log.printf("WiFi: connection lost (%s, %u)\n", reasonText(reason), reason);
        return;
    }

    m_failedAttempts++;

    // While offline, only log when the reason changes so a long outage stays quiet
    if (reason != m_loggedReason)
    {
        m_loggedReason = reason;
        Log.printf("WiFi: connect failed (%s, %u)\n", reasonText(reason), reason);
    }
}

void WiFiDriver::handleConnect()
{
    String took = durationText(millis() - m_offlineSinceMs);
    String ip   = WiFi.localIP().toString();

    if (m_hasConnected)
    {
        Log.printf("WiFi: reconnected after %s, %lu failed attempts (%s, %d dBm)\n",
            took.c_str(), (unsigned long)m_failedAttempts, ip.c_str(), WiFi.RSSI());
    }
    else
    {
        Log.printf("WiFi: connected in %s (%s, %d dBm)\n", took.c_str(), ip.c_str(), WiFi.RSSI());
    }

    m_online       = true;
    m_hasConnected = true;
    m_loggedReason = 0;
}
