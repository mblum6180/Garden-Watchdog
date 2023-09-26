// ESP32 Libraries
#include <WiFi.h>

// Custom Libraries
#include "secrets.h"
#include "ThingSpeak.h"
#include "LEDManager.h"
#include "TemperatureSensor.h"

// Network and ThingSpeak configuration
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient client;
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

#define uS_TO_S_FACTOR 1000000ull
#define TIME_TO_SLEEP 3600



void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  
  delay(5000);

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

