#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <Arduino.h>

class LEDDriver {
private:
    const uint8_t* m_pins = nullptr;
    uint8_t m_numOfLEDs = 0;
public:
    void init(const uint8_t* pins, uint8_t numOfLEDs);

    void setLEDState(uint8_t ledIndex, bool state);
};

#endif