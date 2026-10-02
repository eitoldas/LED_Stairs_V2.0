#pragma once

#include <Arduino.h>

class CurrentSensor
{
public:
    void init(uint8_t pin, float loadVoltage);
    void update();

    float amps() const;
    float watts() const;
    float loadVoltage() const;

    uint32_t millivolts() const;

private:
    void sample();

    uint8_t  m_pin         = 0;
    float    m_loadVoltage = 0.0f;

    uint32_t m_millivolts   = 0;
    float    m_amps         = 0.0f;
    uint32_t m_lastSampleMs = 0;
};
