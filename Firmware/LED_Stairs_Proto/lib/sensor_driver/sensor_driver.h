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
        SensorCallback callback);

    void update();
    void clearPending();

private:
    static void IRAM_ATTR sensorAISR();
    static void IRAM_ATTR sensorBISR();

    static SensorCallback m_callback;

    static uint32_t m_cooldown;

    static volatile bool m_irqPendingA;
    static volatile bool m_irqPendingB;

    static volatile uint32_t m_lastTriggerA;
    static volatile uint32_t m_lastTriggerB;
};

#endif // SENSOR_DRIVER_H
