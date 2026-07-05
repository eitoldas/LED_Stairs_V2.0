#include "led_driver.h"

void LEDDriver::init(const uint8_t* pins, uint8_t numOfLEDs)
{
    m_pins = pins;
    m_numOfLEDs = numOfLEDs;

    for (uint8_t i = 0; i < m_numOfLEDs; i++)
    {
        pinMode(m_pins[i], OUTPUT);
        digitalWrite(m_pins[i], LOW);
    }
}

void LEDDriver::setLEDState(uint8_t ledIndex, bool state)
{
    if (ledIndex >= m_numOfLEDs)
    {
        return;
    }

    digitalWrite(m_pins[ledIndex], state ? HIGH : LOW);
}