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
#define NUM_LEDS 1
#define DATA_PIN 25
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  configureDeepSleep();
  configureWS2812BLed();

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  float fahrenheit = readTemperatureSensor();
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
  if (!ds.search(addr)) {
    Serial.println("No more addresses.");
    ds.reset_search();
    delay(250);
  }

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

  int16_t raw = (data[1] << 8) | data[0];
  float celsius = raw / 16.0;
  float fahrenheit = celsius * 1.8 + 32.0;

  Serial.print("Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");

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

void setLedColorBasedOnTemperature(float fahrenheit) {

  if (fahrenheit > 60) {
    setColor(CRGB::Green); // Green for warm water
  } else if (fahrenheit >= 50 && fahrenheit <= 60) {
    setColor(CRGB::Yellow); // Yellow for cool water
  } else if (fahrenheit < 50 && fahrenheit >= 40) {
    setColor(CRGB::Blue); // Blue for cold water
  } else {
    setColor(CRGB::Red); // Red for water below 40
  }
}

void setColor(CRGB color) {
  leds[0] = color;
  FastLED.show();
}