#pragma once

#include "ntp_driver.h"
#include "wifi_driver.h"

class NetworkManager
{
public:
    void init();
    void update();

    bool isLightsAllowed() const;
    bool isTimeSynced() const;

private:
    bool isWithinNightWindow(const struct tm& timeinfo) const;

    WiFiDriver m_wifiDriver;
    NTPDriver  m_ntpDriver;
};
