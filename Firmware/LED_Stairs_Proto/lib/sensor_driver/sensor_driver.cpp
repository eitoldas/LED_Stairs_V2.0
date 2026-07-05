#include <Arduino.h>

#include "sensor_driver.h"

SensorCallback SensorDriver::m_callback = nullptr;

uint32_t SensorDriver::m_cooldown = 0;

volatile bool     SensorDriver::m_irqPendingA = false;
volatile bool     SensorDriver::m_irqPendingB = false;
volatile uint32_t SensorDriver::m_lastTriggerA = 0;
volatile uint32_t SensorDriver::m_lastTriggerB = 0;

void SensorDriver::init(uint8_t sensorA,
                        uint8_t sensorB,
                        uint32_t cooldown,
                        SensorCallback callback)
{
    m_callback = callback;
    m_cooldown = cooldown;

    m_irqPendingA = false;
    m_irqPendingB = false;
    m_lastTriggerA = 0;
    m_lastTriggerB = 0;

    pinMode(sensorA, INPUT);
    pinMode(sensorB, INPUT);

    attachInterrupt(digitalPinToInterrupt(sensorA), sensorAISR, RISING);
    attachInterrupt(digitalPinToInterrupt(sensorB), sensorBISR, RISING);
}

void SensorDriver::update()
{
    if (m_irqPendingA)
    {
        m_irqPendingA = false;

        uint32_t now = millis();
        if (now - m_lastTriggerA >= m_cooldown)
        {
            m_lastTriggerA = now;
            if (m_callback)
            {
                m_callback(SensorID::SENSOR_A);
            }
        }
    }

    if (m_irqPendingB)
    {
        m_irqPendingB = false;

        uint32_t now = millis();
        if (now - m_lastTriggerB >= m_cooldown)
        {
            m_lastTriggerB = now;
            if (m_callback)
            {
                m_callback(SensorID::SENSOR_B);
            }
        }
    }
}

void SensorDriver::clearPending()
{
    m_irqPendingA = false;
    m_irqPendingB = false;
}

void IRAM_ATTR SensorDriver::sensorAISR()
{
    m_irqPendingA = true;
}

void IRAM_ATTR SensorDriver::sensorBISR()
{
    m_irqPendingB = true;
}
