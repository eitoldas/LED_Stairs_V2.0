#pragma once

#include "ntp_driver.h"
#include "sun_times_driver.h"
#include "wifi_driver.h"

class NetworkManager
{
public:
    void init();
    void update();

    bool isLightsAllowed() const;
    bool isTimeSynced() const;

private:
    bool isRefreshDue() const;
    void refresh();

    bool isWithinNightWindow(const struct tm& timeinfo) const;

    WiFiDriver     m_wifiDriver;
    NTPDriver      m_ntpDriver;
    SunTimesDriver m_sunTimesDriver;

    int      m_lastRefreshDay = -1;
    bool     m_hasAttempted   = false;
    uint32_t m_lastAttemptMs  = 0;
};
