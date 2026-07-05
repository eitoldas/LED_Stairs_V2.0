#include <Arduino.h>

#include "config.h"
#include "stairs_manager.h"

StairsManager* StairsManager::instance = nullptr;

void StairsManager::init()
{
    instance = this;
    Serial.begin(115200);

    ledDriver.init(LED_PINS, NUM_LEDS);
    animationEngine.init(&ledDriver, NUM_LEDS);

    sensorDriver.init(SENSOR_A_PIN, SENSOR_B_PIN, RETRIGGER_COOLDOWN_MS, sensorTriggered);

    animationEngine.startStartup();
}

void StairsManager::update()
{
    animationEngine.update();

    if (m_sensorsBlocked && animationEngine.isStartupComplete())
    {
        m_sensorsBlocked = false;
    }

    if (m_sensorsBlocked)
    {
        return;
    }

    if (m_animationPending)
    {
        m_animationPending = false;

        AnimationDirection direction = (m_pendingSensor == SensorID::SENSOR_A)
            ? AnimationDirection::Up
            : AnimationDirection::Down;

        animationEngine.scheduleRun(direction, millis());
    }
}

void StairsManager::sensorTriggered(SensorID sensor)
{
    // ISR context — flag only, no Serial or other heavy work
    if (instance == nullptr || instance->m_sensorsBlocked)
    {
        return;
    }

    instance->m_pendingSensor    = sensor;
    instance->m_animationPending = true;
}
