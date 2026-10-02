#pragma once

#include <Arduino.h>
#include <time.h>

class NTPDriver
{
public:
    void init(const char* server, const char* timezone);

    bool sync();

    bool isSynced() const;
    bool getLocalTime(struct tm* timeinfo) const;

private:
    const char* m_server   = nullptr;
    const char* m_timezone = nullptr;
    bool        m_synced   = false;
};
