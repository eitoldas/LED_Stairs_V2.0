#pragma once

#include <Arduino.h>

class SunTimesDriver
{
public:
    void init(double latitude, double longitude);

    bool fetch();

    bool     hasTimes() const;
    uint16_t sunriseMinutes() const;
    uint16_t sunsetMinutes() const;

private:
    double m_latitude  = 0.0;
    double m_longitude = 0.0;

    bool     m_hasTimes       = false;
    uint16_t m_sunriseMinutes = 0;
    uint16_t m_sunsetMinutes  = 0;
};
