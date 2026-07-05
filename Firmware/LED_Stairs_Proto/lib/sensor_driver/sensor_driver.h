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
    private:
    static void IRAM_ATTR sensorAISR();
    static void IRAM_ATTR sensorBISR();

    static void handleInterrupt(SensorID sensor);

    static SensorCallback m_callback;

    static uint32_t m_cooldown;

    static volatile uint32_t m_lastTriggerA;
    static volatile uint32_t m_lastTriggerB;
public:
    void init(
        uint8_t sensorA,
        uint8_t sensorB,
        uint32_t cooldown,
        SensorCallback callback);
};

#endif // SENSOR_DRIVER_H