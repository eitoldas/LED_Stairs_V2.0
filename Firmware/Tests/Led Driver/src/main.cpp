#include <Arduino.h>
#include "LEDDriver.h"
#include "LEDModule.h"

// Pin mapping (given by you)
const uint8_t ledPins[14] = {
    1, 4, 5, 6, 7, 15, 16,
    3, 9, 10, 11, 12, 13, 14};

const size_t NUM_LEDS = 14;

// Sensors (PIR) pins
const uint8_t PIR_A = 17;
const uint8_t PIR_B = 18;

// forward declarations for ISRs
void IRAM_ATTR pirA_isr();
void IRAM_ATTR pirB_isr();

// volatile flags set by ISRs, consumed in loop()
volatile bool pirA_flag = false;
volatile bool pirB_flag = false;

// Create driver and module instances (driver stores brightness internally)
LEDDriver ledDriver(ledPins, NUM_LEDS);
LEDModule ledModule(ledDriver);

void setup()
{
  // If you want UART debugging over alternate pins, set up Serial1 here:
  // Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  // Configure PIR pins as input
  pinMode(PIR_A, INPUT);
  pinMode(PIR_B, INPUT);

  // Attach interrupts (RISING). We keep ISRs minimal: set a flag only.
  attachInterrupt(digitalPinToInterrupt(PIR_A), pirA_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(PIR_B), pirB_isr, RISING);

  // Start LED driver (frame time 5 ms -> unit ~= 19.6 us)
  if (!ledDriver.begin(5000))
  {
    // initialization failed — handle as you see fit
    // (no Serial available on pins 1/3 in this wiring)
  }

  ledModule.begin();

  // Kick off a slow wave so the stairs are visible on boot
  ledModule.startAnimation(LEDModule::ANIM_WAVE, 80, 10, 160);
}

void loop()
{
  // check PIR triggers (set by ISR)
  if (pirA_flag)
  {
    pirA_flag = false;
    // start chase animation for 4 seconds
    ledModule.triggerSensor(LEDModule::ANIM_CHASE, 4000);
  }
  if (pirB_flag)
  {
    pirB_flag = false;
    // wave but faster for 6 seconds
    ledModule.triggerSensor(LEDModule::ANIM_WAVE, 6000);
  }

  // non-blocking tick for module
  ledModule.tick();

  // keep loop light; driver runs the PWM in timer callbacks
  delay(5);
}

// ISRs — keep minimal and IRAM safe
void IRAM_ATTR pirA_isr()
{
  pirA_flag = true;
}
void IRAM_ATTR pirB_isr()
{
  pirB_flag = true;
}
