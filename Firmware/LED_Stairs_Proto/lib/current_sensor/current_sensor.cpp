#include <Arduino.h>

#include "config.h"
#include "current_sensor.h"

void CurrentSensor::init(uint8_t pin, float loadVoltage)
{
    m_pin         = pin;
    m_loadVoltage = loadVoltage;

    analogSetPinAttenuation(m_pin, ADC_11db);

    sample();
    m_lastSampleMs = millis();
}

void CurrentSensor::update()
{
    uint32_t now = millis();
    if (now - m_lastSampleMs < CURRENT_SAMPLE_INTERVAL_MS)
    {
        return;
    }

    m_lastSampleMs = now;
    sample();
}

float CurrentSensor::amps() const
{
    return m_amps;
}

float CurrentSensor::watts() const
{
    return m_amps * m_loadVoltage;
}

float CurrentSensor::loadVoltage() const
{
    return m_loadVoltage;
}

uint32_t CurrentSensor::millivolts() const
{
    return m_millivolts;
}

void CurrentSensor::sample()
{
    uint32_t total = 0;

    for (uint8_t i = 0; i < CURRENT_SAMPLE_COUNT; i++)
    {
        total += analogReadMilliVolts(m_pin);
    }

    m_millivolts = total / CURRENT_SAMPLE_COUNT;

    // VIOUT falls as load current rises with the sensor as fitted, so the
    // zero-current level is subtracted the other way round.
    m_amps       = (CURRENT_ZERO_MV - (int32_t)m_millivolts)
                 / (float)CURRENT_SENSITIVITY_MV_PER_A;
}
