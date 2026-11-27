#pragma once

#include <Arduino.h>

/*
 * Keeps the board on WiFi and logs what happens to the connection: why it
 * dropped, failed attempts while offline, and how long it took to come back.
 */

// The board's address on the network. A zero ip means DHCP, and a zero dns
// falls back to the gateway.
struct WiFiAddress
{
    uint32_t ip      = 0;
    uint32_t gateway = 0;
    uint32_t subnet  = 0;
    uint32_t dns     = 0;

    bool operator==(const WiFiAddress& other) const
    {
        return ip == other.ip && gateway == other.gateway
            && subnet == other.subnet && dns == other.dns;
    }
};

class WiFiDriver
{
public:
    void init(const char* ssid, const char* password, const WiFiAddress& address);
    void update();

    // Reconnects straight away using the new address
    void setAddress(const WiFiAddress& address);

    // True once the connection has been logged, so everything that waits on
    // WiFi starts after the "connected" line
    bool isConnected() const;

private:
    void applyAddress();
    void handleDisconnect(uint8_t reason);
    void handleConnect();

    const char* m_ssid     = nullptr;
    const char* m_password = nullptr;
    WiFiAddress m_address;
    uint32_t    m_lastAttemptMs = 0;

    bool     m_online         = false;
    bool     m_hasConnected   = false;
    uint32_t m_offlineSinceMs = 0;
    uint32_t m_failedAttempts = 0;
    uint8_t  m_loggedReason   = 0;

    // Written by the WiFi event task, read in update()
    volatile uint8_t m_disconnectReason  = 0;
    volatile bool    m_disconnectPending = false;
    volatile bool    m_connectPending    = false;
};
