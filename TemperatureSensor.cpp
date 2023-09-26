#include "TemperatureSensor.h"
#include <OneWire.h>

// DS18x20 temperature sensor configuration
OneWire ds(14);


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

