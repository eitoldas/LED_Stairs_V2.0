#include <Arduino.h>

#include "sensor_driver.h"

SensorCallback SensorDriver::m_callback = nullptr;

uint32_t SensorDriver::m_cooldown = 0;
uint32_t SensorDriver::m_confirm  = 0;

uint8_t SensorDriver::m_pinA = 0;
uint8_t SensorDriver::m_pinB = 0;

volatile bool     SensorDriver::m_irqPendingA = false;
volatile bool     SensorDriver::m_irqPendingB = false;
volatile uint32_t SensorDriver::m_edgeMsA     = 0;
volatile uint32_t SensorDriver::m_edgeMsB     = 0;

uint32_t SensorDriver::m_lastTriggerA = 0;
uint32_t SensorDriver::m_lastTriggerB = 0;

void SensorDriver::init(uint8_t sensorA,
                        uint8_t sensorB,
                        uint32_t cooldown,
                        uint32_t confirm,
                        SensorCallback callback)
{
    m_callback = callback;
    m_cooldown = cooldown;
    m_confirm  = confirm;
    m_pinA     = sensorA;
    m_pinB     = sensorB;

    m_irqPendingA  = false;
    m_irqPendingB  = false;
    m_lastTriggerA = 0;
    m_lastTriggerB = 0;

    pinMode(sensorA, INPUT_PULLDOWN);
    pinMode(sensorB, INPUT_PULLDOWN);

    attachInterrupt(digitalPinToInterrupt(sensorA), sensorAISR, RISING);
    attachInterrupt(digitalPinToInterrupt(sensorB), sensorBISR, RISING);
}

void SensorDriver::update()
{
    checkSensor(SensorID::SENSOR_A, m_pinA, m_irqPendingA, m_edgeMsA, m_lastTriggerA);
    checkSensor(SensorID::SENSOR_B, m_pinB, m_irqPendingB, m_edgeMsB, m_lastTriggerB);
}

void SensorDriver::checkSensor(SensorID sensor,
                               uint8_t pin,
                               volatile bool& irqPending,
                               volatile uint32_t& edgeMs,
                               uint32_t& lastTrigger)
{
    if (!irqPending)
    {
        return;
    }

    // A real PIR holds its output high for seconds. Electrical noise, like a
    // gas hob igniter, only produces a brief spike, so the pin has to still
    // be high once the confirm time has passed before it counts.
    if (digitalRead(pin) == LOW)
    {
        irqPending = false;
        Serial.printf("Sensor %c: ignored noise spike\n", sensor == SensorID::SENSOR_A ? 'A' : 'B');
        return;
    }

    uint32_t now = millis();
    if (now - edgeMs < m_confirm)
    {
        return;
    }

    irqPending = false;

    if (now - lastTrigger < m_cooldown)
    {
        return;
    }

    lastTrigger = now;
    if (m_callback)
    {
        m_callback(sensor);
    }
}

void SensorDriver::clearPending()
{
    m_irqPendingA = false;
    m_irqPendingB = false;
}

void IRAM_ATTR SensorDriver::sensorAISR()
{
    m_edgeMsA     = millis();
    m_irqPendingA = true;
}

void IRAM_ATTR SensorDriver::sensorBISR()
{
    m_edgeMsB     = millis();
    m_irqPendingB = true;
}
