#pragma once

#include "led_driver.h"
#include "sensor_driver.h"
#include "config.h"

struct LEDSchedule
{
    uint32_t turnOnAt  = 0;
    uint32_t turnOffAt = 0;
    bool     active    = false;
};

class StairsManager
{
public:
    void init();
    void update();

private:
    static void sensorTriggered(SensorID sensor);
    void scheduleAnimation(SensorID direction);

    LEDDriver    ledDriver;
    SensorDriver sensorDriver;

    LEDSchedule m_schedules[NUM_LEDS];

    volatile bool     m_animationPending = false;
    volatile SensorID m_pendingSensor    = SensorID::SENSOR_A;

    static StairsManager* instance;
};