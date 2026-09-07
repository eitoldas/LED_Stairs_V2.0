#include <Arduino.h>

#include "animation_engine.h"

void AnimationEngine::init(LEDDriver* ledDriver, uint8_t numLeds)
{
    m_ledDriver = ledDriver;
    m_numLeds   = numLeds;

    for (uint8_t i = 0; i < m_numLeds; i++)
    {
        m_schedules[i] = {};
    }
}

void AnimationEngine::update()
{
    uint32_t now = millis();

    if (m_startupState == StartupState::Running)
    {
        updateStartup(now);
    }

    updateSchedules(now);
}

void AnimationEngine::scheduleRun(AnimationDirection direction, uint32_t now)
{
    for (uint8_t i = 0; i < m_numLeds; i++)
    {
        uint8_t led = (direction == AnimationDirection::Up)
            ? i
            : (m_numLeds - 1 - i);

        uint32_t turnOnAt = now + (uint32_t)i * ANIM_STEP_DELAY_MS;
        uint32_t turnOffAt = now
            + (uint32_t)(m_numLeds - 1) * ANIM_STEP_DELAY_MS
            + STAIR_HOLD_DELAY_MS
            + (uint32_t)i * ANIM_STEP_DELAY_MS;

        mergeSchedule(led, turnOnAt, turnOffAt);
    }
}

void AnimationEngine::scheduleBlink(uint32_t now, uint16_t intervalMs, uint8_t count)
{
    for (uint8_t b = 0; b < count; b++)
    {
        uint32_t onAt  = now + (uint32_t)b * 2 * intervalMs;
        uint32_t offAt = onAt + intervalMs;

        for (uint8_t led = 0; led < m_numLeds; led++)
        {
            mergeSchedule(led, onAt, offAt);
        }
    }
}

void AnimationEngine::startStartup()
{
    m_startupState = StartupState::Running;
    m_startupPhase = StartupPhase::Approach;
    m_phaseStartMs = millis();
    m_startupStep  = 0;
    m_blinkCount   = 0;
    m_blinkOn      = false;

    for (uint8_t i = 0; i < m_numLeds; i++)
    {
        m_schedules[i] = {};
    }

    setAllOff();
}

bool AnimationEngine::isStartupComplete() const
{
    return m_startupState == StartupState::Complete;
}

bool AnimationEngine::isRunActive() const
{
    for (uint8_t i = 0; i < m_numLeds; i++)
    {
        if (m_schedules[i].active)
        {
            return true;
        }
    }

    return false;
}

void AnimationEngine::mergeSchedule(uint8_t led, uint32_t turnOnAt, uint32_t turnOffAt)
{
    if (led >= m_numLeds)
    {
        return;
    }

    if (m_schedules[led].active)
    {
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

void AnimationEngine::updateSchedules(uint32_t now)
{
    for (uint8_t i = 0; i < m_numLeds; i++)
    {
        if (!m_schedules[i].active)
        {
            continue;
        }

        if (now >= m_schedules[i].turnOffAt)
        {
            m_ledDriver->setLEDState(i, false);
            m_schedules[i].active = false;
        }
        else if (now >= m_schedules[i].turnOnAt)
        {
            m_ledDriver->setLEDState(i, true);
        }
    }
}

void AnimationEngine::updateStartup(uint32_t now)
{
    switch (m_startupPhase)
    {
        case StartupPhase::Approach:
            advanceApproach(now);
            break;

        case StartupPhase::Expand:
            advanceExpand(now);
            break;

        case StartupPhase::Hold:
            if (now - m_phaseStartMs >= STARTUP_HOLD_MS)
            {
                m_startupPhase = StartupPhase::Blink;
                m_phaseStartMs = millis();
                m_blinkCount   = 0;
                m_blinkOn      = false;
                setAllOff();
            }
            break;

        case StartupPhase::Blink:
        {
            if (now - m_phaseStartMs >= STARTUP_BLINK_INTERVAL_MS)
            {
                m_phaseStartMs = now;
                m_blinkOn = !m_blinkOn;

                for (uint8_t i = 0; i < m_numLeds; i++)
                {
                    m_ledDriver->setLEDState(i, m_blinkOn);
                }

                if (!m_blinkOn)
                {
                    m_blinkCount++;
                }

                if (m_blinkCount >= STARTUP_BLINK_COUNT)
                {
                    setAllOff();
                    m_startupPhase = StartupPhase::Done;
                    m_startupState = StartupState::Complete;
                }
            }
            break;
        }

        case StartupPhase::Done:
            break;
    }
}

void AnimationEngine::setAllOff()
{
    for (uint8_t i = 0; i < m_numLeds; i++)
    {
        m_ledDriver->setLEDState(i, false);
    }
}

void AnimationEngine::advanceApproach(uint32_t now)
{
    if (now - m_phaseStartMs < STARTUP_STEP_DELAY_MS)
    {
        return;
    }

    m_phaseStartMs = now;

    if (m_startupStep > 0)
    {
        uint8_t prev = m_startupStep - 1;
        m_ledDriver->setLEDState(prev, false);
        m_ledDriver->setLEDState(m_numLeds - 1 - prev, false);
    }

    m_ledDriver->setLEDState(m_startupStep, true);
    m_ledDriver->setLEDState(m_numLeds - 1 - m_startupStep, true);

    m_startupStep++;

    if (m_startupStep >= (m_numLeds + 1) / 2)
    {
        m_startupPhase = StartupPhase::Expand;
        m_phaseStartMs = now;
        m_startupStep  = 0;
    }
}

void AnimationEngine::advanceExpand(uint32_t now)
{
    if (now - m_phaseStartMs < STARTUP_STEP_DELAY_MS)
    {
        return;
    }

    m_phaseStartMs = now;

    uint8_t left  = (m_numLeds / 2) - 2 - m_startupStep;
    uint8_t right = (m_numLeds / 2) + 1 + m_startupStep;

    if (left < m_numLeds)
    {
        m_ledDriver->setLEDState(left, true);
    }
    if (right < m_numLeds)
    {
        m_ledDriver->setLEDState(right, true);
    }

    m_startupStep++;

    if (m_startupStep >= (m_numLeds + 1) / 2 - 1)
    {
        for (uint8_t i = 0; i < m_numLeds; i++)
        {
            m_ledDriver->setLEDState(i, true);
        }

        m_startupPhase = StartupPhase::Hold;
        m_phaseStartMs = now;
    }
}
