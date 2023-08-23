//esp32
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h"
#include <OneWire.h>
#include <FastLED.h>

// Network and ThingSpeak configuration
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient client;
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

// DS18x20 temperature sensor configuration
OneWire ds(14);
#define uS_TO_S_FACTOR 1000000ull
#define TIME_TO_SLEEP 3600

// WS2812B LED strip configuration
#define NUM_LEDS 9
#define DATA_PIN 12
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  
  configureDeepSleep();
  configureWS2812BLed();

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  float fahrenheit = readTemperatureSensor();
  //fahrenheit = 90; // for testing leds
  setLedColorBasedOnTemperature(fahrenheit);
  connectToWiFi();
  updateThingSpeakChannel(fahrenheit);

  goToDeepSleep();
}

void loop() {
  // Empty as the code runs only in the setup() function
}

void configureDeepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
}

float readTemperatureSensor() {
  byte addr[8];
  int retryCount = 0;
  const int maxRetries = 10;
  float fahrenheit = -1000.0;

  while (retryCount < maxRetries && fahrenheit == -1000.0) {
    if (!ds.search(addr)) {
      Serial.println("No more addresses.");
      ds.reset_search();
      delay(250);
    } else if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("Address CRC is not valid!");
    } else {
      identifySensor(addr);

      ds.reset();
      ds.select(addr);
      ds.write(0x44, 1);
      delay(750);

      ds.reset();
      ds.select(addr);
      ds.write(0xBE);

      byte data[9];
      for (byte i = 0; i < 9; i++) {
        data[i] = ds.read();
      }

      if (OneWire::crc8(data, 8) != data[8]) {
        Serial.println("Data CRC is not valid!");
      } else {
        int16_t raw = (data[1] << 8) | data[0];
        float celsius = raw / 16.0;
        fahrenheit = celsius * 1.8 + 32.0;

        Serial.print("Temperature = ");
        Serial.print(celsius);
        Serial.print(" Celsius, ");
        Serial.print(fahrenheit);
        Serial.println(" Fahrenheit");
      }
    }

    retryCount++;
    if (fahrenheit == -1000.0) {
      delay(250); // Wait before retrying
    }
  }

  if (fahrenheit == -1000.0) {
    Serial.println("Failed to read a valid temperature after maximum retries.");
  }

  return fahrenheit;
}

void identifySensor(byte addr[]) {
  byte type_s;

  Serial.print("ROM =");
  for (byte i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  Serial.println();

  // The first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }
}


void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
}

void updateThingSpeakChannel(float fahrenheit) {
  if (fahrenheit == -1000.0) {
    Serial.println("Sensor reading is not valid. Skipping channel update.");
    return;
  }

  int response = ThingSpeak.writeField(myChannelNumber, 1, fahrenheit, myWriteAPIKey);
  if (response == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(response));
  }
}


void goToDeepSleep() {
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

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
    scaledFactor = (tempScaled - 51) * 5;
    scaledFactor = constrain(scaledFactor, 0, 255);  // ensure the value is between 0 and 255
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
