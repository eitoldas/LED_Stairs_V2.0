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

    uint32_t now = millis();

    // The far sensor trips as the sweep finishes, so a trigger from it around
    // that moment is the same walker leaving rather than a new one arriving.
    if (animationEngine.isRunActive()
        && m_pendingSensor != m_runSensor
        && isWithinExitWindow(now))
    {
        return;
    }

    if (m_networkManager != nullptr && !m_networkManager->isLightsAllowed())
    {
        return;
    }

    m_runSensor    = m_pendingSensor;
    m_runStartedAt = now;

    AnimationDirection direction = (m_pendingSensor == SensorID::SENSOR_A)
        ? AnimationDirection::Up
        : AnimationDirection::Down;

    animationEngine.scheduleRun(direction, now);
}

bool StairsManager::isWithinExitWindow(uint32_t now) const
{
    uint32_t expectedAt = m_runStartedAt + animationEngine.sweepDurationMs();
    int32_t  offset     = (int32_t)(now - expectedAt);

    return abs(offset) <= (int32_t)EXIT_OFFSET_MS;
}

void StairsManager::sensorTriggered(SensorID sensor)
{
    instance->m_pendingSensor    = sensor;
    instance->m_animationPending = true;
}
