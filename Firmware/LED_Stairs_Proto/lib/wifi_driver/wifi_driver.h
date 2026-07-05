#pragma once

#include <Arduino.h>

class WiFiDriver
{
public:
    void init(const char* ssid, const char* password);
    void update();

    bool isConnected() const;

private:
    const char* m_ssid     = nullptr;
    const char* m_password = nullptr;
    uint32_t    m_lastAttemptMs = 0;
};
