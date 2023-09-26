#include "LEDManager.h"

// WS2812B LED strip configuration
#define NUM_LEDS 9
#define DATA_PIN 12 //12
CRGB leds[NUM_LEDS];

void configureWS2812BLed() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

CRGB determineColor(float fahrenheit) {
  // Define temperature extremes
  const float tempMin = 35.0;
  const float tempMax = 80.0;

  // Clip temperature values to our defined minimum and maximum
  float tempClipped = min(max(fahrenheit, tempMin), tempMax);

  // Scale temperature to a value between 0 and 255
  uint8_t tempScaled = map(tempClipped, tempMin, tempMax, 0, 255);

  // Define color ranges
  CRGB colorCold = CRGB::Blue;       // Cold water
  CRGB colorCool = CRGB::Cyan;       // Cool water
  CRGB colorNeutral = CRGB::Green;   // Neutral water
  CRGB colorWarm = CRGB::Yellow;     // Warm water
  CRGB colorHot = CRGB::Red;         // Hot water

  uint8_t r, g, b;
  uint8_t scaledFactor;
  if (tempScaled < 51) {  // Lower part of the temperature range (cold to neutral)
    scaledFactor = tempScaled * 5;
    r = lerp8by8(colorCold.r, colorNeutral.r, scaledFactor);
    g = lerp8by8(colorCold.g, colorNeutral.g, scaledFactor);
    b = lerp8by8(colorCold.b, colorNeutral.b, scaledFactor);
  } else {  // Upper part of the temperature range (neutral to hot)
    scaledFactor = ((tempScaled - 51) * 255) / (255 - 51);  // ensure the value is between 0 and 255
    r = lerp8by8(colorNeutral.r, colorHot.r, scaledFactor);
    g = lerp8by8(colorNeutral.g, colorHot.g, scaledFactor);
    b = lerp8by8(colorNeutral.b, colorHot.b, scaledFactor);
  }

  CRGB color = CRGB(r, g, b);

  return color;
}

void setLedColorBasedOnTemperature(float fahrenheit) {
  CRGB color = determineColor(fahrenheit);
  setColor(color);
}


void setColor(CRGB color) {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}
