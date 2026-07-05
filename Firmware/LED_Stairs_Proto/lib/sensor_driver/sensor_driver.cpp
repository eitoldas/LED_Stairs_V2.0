#include <Arduino.h>

#include "sensor_driver.h"

SensorCallback SensorDriver::m_callback = nullptr;

uint32_t SensorDriver::m_cooldown = 0;
volatile uint32_t SensorDriver::m_lastTriggerA = 0;
volatile uint32_t SensorDriver::m_lastTriggerB = 0;


void SensorDriver::init(uint8_t sensorA,
                        uint8_t sensorB,
                        uint32_t cooldown,
                        SensorCallback callback) 
{
    m_callback = callback;
    m_cooldown = cooldown;

    pinMode(sensorA, INPUT);
    pinMode(sensorB, INPUT);

    attachInterrupt(digitalPinToInterrupt(sensorA), sensorAISR, RISING);
    attachInterrupt(digitalPinToInterrupt(sensorB), sensorBISR, RISING);
}

void IRAM_ATTR SensorDriver::sensorAISR() 
{
    handleInterrupt(SensorID::SENSOR_A);
}

void IRAM_ATTR SensorDriver::sensorBISR() 
{
    handleInterrupt(SensorID::SENSOR_B);
}

void SensorDriver::handleInterrupt(SensorID sensor) 
{
    uint32_t currentTime = millis();

    if (sensor == SensorID::SENSOR_A) 
    {
        if (currentTime - m_lastTriggerA >= m_cooldown) 
        {
            m_lastTriggerA = currentTime;
            if (m_callback) 
            {
                m_callback(sensor);
            }
        }
    } 
    else if (sensor == SensorID::SENSOR_B) 
    {
        if (currentTime - m_lastTriggerB >= m_cooldown) 
        {
            m_lastTriggerB = currentTime;
            if (m_callback) 
            {
                m_callback(sensor);
            }
        }
    }
}
