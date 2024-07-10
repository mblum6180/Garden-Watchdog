#include <WiFi.h>
#include "secrets.h" 
#include "ThingSpeak.h"
#include "LEDManager.h"
#include "TemperatureSensor.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient client;
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

#define uS_TO_S_FACTOR 1000000ull
#define TIME_TO_SLEEP 3600

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read(); // Correct declaration for external C function.

#ifdef __cplusplus
}
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  delay(5000);

  configureDeepSleep();
  configureWS2812BLed();

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  float fahrenheit = readTemperatureSensor();
  setLedColorBasedOnTemperature(fahrenheit);
  
  if (connectToWiFi()) {
    uint8_t internalTempF = temprature_sens_read(); // Internal temperature in Fahrenheit
    updateThingSpeakChannel(fahrenheit, internalTempF);
  } else {
    Serial.println("Failed to connect to WiFi. Going to sleep.");
  }

  goToDeepSleep();
}

void loop() {}

void configureDeepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("ESP32 configured to deep sleep for every " + String(TIME_TO_SLEEP) + " seconds.");
}

bool connectToWiFi() {
  Serial.print("Connecting to SSID: " + String(ssid));
  WiFi.begin(ssid, pass);
  
  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED && retry_count < 3) {
    delay(5000);
    Serial.print(".");
    retry_count++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect.");
    return false;
  }

  Serial.println("\nConnected.");
  return true;
}

void updateThingSpeakChannel(float fahrenheit, uint8_t internalTempF) {
  ThingSpeak.setField(1, fahrenheit);         // External temperature
  ThingSpeak.setField(2, internalTempF);      // Internal temperature
  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  if (response == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(response));
  }
}

void goToDeepSleep() {
  Serial.println("Going to sleep now.");
  Serial.flush();
  esp_deep_sleep_start();
}
