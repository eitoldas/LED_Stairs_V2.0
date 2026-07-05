#pragma once

#include "animation_engine.h"
#include "led_driver.h"
#include "network_manager.h"
#include "sensor_driver.h"
#include "config.h"

class StairsManager
{
public:
    void init(NetworkManager* networkManager);
    void update();

private:
    static void sensorTriggered(SensorID sensor);

    NetworkManager*  m_networkManager = nullptr;
    LEDDriver        ledDriver;
    SensorDriver     sensorDriver;
    AnimationEngine  animationEngine;

    volatile bool     m_animationPending = false;
    volatile SensorID m_pendingSensor    = SensorID::SENSOR_A;
    volatile bool     m_sensorsBlocked   = true;

    static StairsManager* instance;
};
