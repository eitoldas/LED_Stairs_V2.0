#include <Arduino.h>

#include "config.h"
#include "stairs_manager.h"

StairsManager* StairsManager::instance = nullptr;

void StairsManager::init()
{
    instance = this;
    ledDriver.init(LED_PINS, NUM_LEDS);
    sensorDriver.init(SENSOR_A_PIN, SENSOR_B_PIN, RETRIGGER_COOLDOWN_MS, sensorTriggered);
    Serial.begin(115200);
}

void StairsManager::update()
{
    // ISR just set a flag — pick it up here and schedule, never in the ISR
    if (m_animationPending)
    {
        m_animationPending = false;
        scheduleAnimation(m_pendingSensor);
    }

    // Drive LEDs — this is the only place we touch hardware
    uint32_t now = millis();
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
        if (!m_schedules[i].active) continue;

        if (now >= m_schedules[i].turnOffAt)
        {
            ledDriver.setLEDState(i, false);
            m_schedules[i].active = false;
        }
        else if (now >= m_schedules[i].turnOnAt)
        {
            ledDriver.setLEDState(i, true);
        }
    }
}

void StairsManager::sensorTriggered(SensorID sensor)
{
    // ISR context — flag only, no work
    instance->m_pendingSensor    = sensor;
    instance->m_animationPending = true;
}

void StairsManager::scheduleAnimation(SensorID direction)
{
    uint32_t now = millis();

    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
        // SENSOR_A (up): LED 0 first → LED 13 last
        // SENSOR_B (down): LED 13 first → LED 0 last
        uint8_t led = (direction == SensorID::SENSOR_A) ? i : (NUM_LEDS - 1 - i);

        uint32_t turnOnAt  = now + (uint32_t)i * LED_STEP_DELAY_MS;
        uint32_t turnOffAt = turnOnAt + (uint32_t)NUM_LEDS * LED_STEP_DELAY_MS;

        if (m_schedules[led].active)
        {
            // Merge: light up as early as either animation needs,
            // stay on as late as either animation needs
            if (turnOnAt  < m_schedules[led].turnOnAt)  m_schedules[led].turnOnAt  = turnOnAt;
            if (turnOffAt > m_schedules[led].turnOffAt) m_schedules[led].turnOffAt = turnOffAt;
        }
        else
        {
            m_schedules[led].turnOnAt  = turnOnAt;
            m_schedules[led].turnOffAt = turnOffAt;
            m_schedules[led].active    = true;
        }
    }
}