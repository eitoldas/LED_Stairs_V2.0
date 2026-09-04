#include <Arduino.h>

#include "config.h"
#include "stairs_manager.h"

StairsManager* StairsManager::instance = nullptr;

void StairsManager::init(NetworkManager* networkManager)
{
    instance = this;
    m_networkManager = networkManager;

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
        sensorDriver.clearPending();
        return;
    }

    sensorDriver.update();

    if (!m_animationPending)
    {
        return;
    }

    m_animationPending = false;

    if (m_networkManager != nullptr && !m_networkManager->isLightsAllowed())
    {
        return;
    }

    AnimationDirection direction = (m_pendingSensor == SensorID::SENSOR_A)
        ? AnimationDirection::Up
        : AnimationDirection::Down;

    animationEngine.scheduleRun(direction, millis());
}

void StairsManager::sensorTriggered(SensorID sensor)
{
    instance->m_pendingSensor    = sensor;
    instance->m_animationPending = true;
}
