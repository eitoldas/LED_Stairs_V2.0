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

    bool isWithinExitWindow(uint32_t now) const;

    NetworkManager*  m_networkManager = nullptr;
    LEDDriver        ledDriver;
    SensorDriver     sensorDriver;
    AnimationEngine  animationEngine;

    volatile bool     m_animationPending = false;
    volatile SensorID m_pendingSensor    = SensorID::SENSOR_A;
    volatile bool     m_sensorsBlocked   = true;

    SensorID m_runSensor    = SensorID::SENSOR_A;
    uint32_t m_runStartedAt = 0;
    bool     m_exitSeen     = false;

    static StairsManager* instance;
};
