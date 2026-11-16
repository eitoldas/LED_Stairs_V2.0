#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <Arduino.h>

enum class SensorID
{
    SENSOR_A,
    SENSOR_B
};

using SensorCallback = void (*)(SensorID sensor);

class SensorDriver
{
public:
    void init(
        uint8_t sensorA,
        uint8_t sensorB,
        uint32_t cooldown,
        uint32_t confirm,
        SensorCallback callback);

    void update();
    void clearPending();

private:
    static void IRAM_ATTR sensorAISR();
    static void IRAM_ATTR sensorBISR();

    static void checkSensor(
        SensorID sensor,
        uint8_t pin,
        volatile bool& irqPending,
        volatile uint32_t& edgeMs,
        uint32_t& lastTrigger);

    static SensorCallback m_callback;

    static uint32_t m_cooldown;
    static uint32_t m_confirm;

    static uint8_t m_pinA;
    static uint8_t m_pinB;

    static volatile bool m_irqPendingA;
    static volatile bool m_irqPendingB;

    static volatile uint32_t m_edgeMsA;
    static volatile uint32_t m_edgeMsB;

    static uint32_t m_lastTriggerA;
    static uint32_t m_lastTriggerB;
};

#endif // SENSOR_DRIVER_H
