#pragma once

#include <Arduino.h>

/*
 * Keeps the board on WiFi and logs what happens to the connection: why it
 * dropped, failed attempts while offline, and how long it took to come back.
 */

class WiFiDriver
{
public:
    void init(const char* ssid, const char* password);
    void update();

    // True once the connection has been logged, so everything that waits on
    // WiFi starts after the "connected" line
    bool isConnected() const;

private:
    void handleDisconnect(uint8_t reason);
    void handleConnect();

    const char* m_ssid     = nullptr;
    const char* m_password = nullptr;
    uint32_t    m_lastAttemptMs = 0;

    bool     m_online         = false;
    bool     m_hasConnected   = false;
    uint32_t m_offlineSinceMs = 0;
    uint32_t m_failedAttempts = 0;
    uint8_t  m_loggedReason   = 0;

    // Written by the WiFi event task, read in update()
    volatile uint8_t m_eventReason  = 0;
    volatile bool    m_eventPending = false;
};
