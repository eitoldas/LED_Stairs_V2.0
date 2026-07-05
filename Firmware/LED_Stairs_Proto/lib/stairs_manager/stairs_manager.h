#pragma once

#include "animation_engine.h"
#include "led_driver.h"
#include "sensor_driver.h"
#include "config.h"

class StairsManager
{
public:
    void init();
    void update();

private:
    static void sensorTriggered(SensorID sensor);

    LEDDriver        ledDriver;
    SensorDriver     sensorDriver;
    AnimationEngine  animationEngine;

    volatile bool     m_animationPending = false;
    volatile SensorID m_pendingSensor    = SensorID::SENSOR_A;
    volatile bool     m_sensorsBlocked   = true;

    static StairsManager* instance;
};
