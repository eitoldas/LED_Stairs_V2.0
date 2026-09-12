#pragma once

#include <Arduino.h>

#include "build_info.h"

class DeviceInfo
{
public:
    void init();

    const char* hardwareVersion() const;
    const char* firmwareVersion() const;
    const char* buildTimestamp() const;

    const char* deviceId() const;
    const char* resetReason() const;

private:
    char        m_deviceId[18] = {};
    const char* m_resetReason  = "Unknown";
};
