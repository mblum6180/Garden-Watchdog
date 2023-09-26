#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <FastLED.h>

// Function declarations
void configureWS2812BLed();
CRGB determineColor(float fahrenheit);
void setLedColorBasedOnTemperature(float fahrenheit);
void setColor(CRGB color);

#endif
