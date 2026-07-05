#pragma once

#include <Arduino.h>

#include "animation_config.h"
#include "config.h"
#include "led_driver.h"

enum class AnimationDirection
{
    Up,
    Down
};

enum class StartupState
{
    NotRunning,
    Running,
    Complete
};

struct LEDSchedule
{
    uint32_t turnOnAt  = 0;
    uint32_t turnOffAt = 0;
    bool     active    = false;
};

class AnimationEngine
{
public:
    void init(LEDDriver* ledDriver, uint8_t numLeds);

    void update();

    void scheduleRun(AnimationDirection direction, uint32_t now);
    void scheduleBlink(uint32_t now, uint16_t intervalMs, uint8_t count);

    void startStartup();

    bool isStartupComplete() const;

private:
    enum class StartupPhase
    {
        Approach,
        Expand,
        Hold,
        Blink,
        Done
    };

    void mergeSchedule(uint8_t led, uint32_t turnOnAt, uint32_t turnOffAt);
    void updateSchedules(uint32_t now);
    void updateStartup(uint32_t now);

    void setAllOff();
    void advanceApproach(uint32_t now);
    void advanceExpand(uint32_t now);

    LEDDriver* m_ledDriver = nullptr;
    uint8_t    m_numLeds   = 0;

    LEDSchedule m_schedules[NUM_LEDS];

    StartupState  m_startupState  = StartupState::NotRunning;
    StartupPhase  m_startupPhase  = StartupPhase::Approach;
    uint32_t      m_phaseStartMs  = 0;
    uint8_t       m_startupStep   = 0;
    uint8_t       m_blinkCount    = 0;
    bool          m_blinkOn       = false;
};
