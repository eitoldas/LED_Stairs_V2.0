#include <Arduino.h>

/*
  ESP32-S3 Software PWM Engine
  - 14 independent outputs
  - Non-blocking
  - Smooth animation
*/

const uint8_t ledPins[14] = {
    1, 4, 5, 6, 7, 15, 16,
    3, 9, 10, 11, 12, 13, 14};

const uint8_t NUM_LEDS = 14;

// Software PWM config
const uint16_t PWM_PERIOD_US = 1000; // 1 kHz PWM
const uint8_t PWM_RES = 255;         // 8-bit resolution

uint8_t brightness[NUM_LEDS]; // 0–255
uint8_t pwmCounter = 0;

uint32_t lastPwmTick = 0;
uint32_t lastAnimTick = 0;

// Animation config
const uint32_t ANIM_INTERVAL_US = 2000; // animation speed
float phase = 0.0f;


void setup()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    brightness[i] = 0;
  }
}


// ---------------- SOFTWARE PWM ----------------
void pwmEngine(uint32_t now)
{
  if (now - lastPwmTick >= (PWM_PERIOD_US / PWM_RES))
  {
    lastPwmTick = now;
    pwmCounter++;

    for (int i = 0; i < NUM_LEDS; i++)
    {
      if (brightness[i] > pwmCounter)
      {
        digitalWrite(ledPins[i], HIGH);
      }
      else
      {
        digitalWrite(ledPins[i], LOW);
      }
    }
  }
}

// ---------------- ANIMATION ----------------
void animationEngine(uint32_t now)
{
  if (now - lastAnimTick >= ANIM_INTERVAL_US)
  {
    lastAnimTick = now;
    phase += 0.005f;

    for (int i = 0; i < NUM_LEDS; i++)
    {
      float x = phase + (i * 0.4f);
      float s = (sinf(x) + 1.0f) * 0.5f; // 0.0–1.0
      brightness[i] = (uint8_t)(s * 255);
    }
  }
}

// ---------------- MAIN LOOP ----------------
void loop()
{
  uint32_t now = micros();

  pwmEngine(now);
  animationEngine(now);
}
