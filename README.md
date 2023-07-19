# ESP32 Pond Water Temperature Monitor

This is a simple IoT project that utilizes an ESP32, a DS18B20 temperature sensor, and a WS2812B LED strip to monitor and display the water temperature of a pond.

## Components

- ESP32
- DS18B20 temperature sensor
- WS2812B LED strip
- Resistors and capacitors for connecting the sensor and LED strip (if necessary)

## Functionality

The ESP32 reads the temperature from the DS18B20 sensor and sets the color of the LEDs on the WS2812B strip based on the temperature reading. The color transitions from blue (for cold water) through cyan (cool water), green (neutral water), yellow (warm water), to red (for hot water) as the temperature changes.

Additionally, the ESP32 connects to a WiFi network and updates a ThingSpeak channel with the current temperature reading.

After updating the ThingSpeak channel, the ESP32 goes into deep sleep mode for a certain period to conserve power. After the sleep period, the ESP32 wakes up and repeats the process.

## Installation

1. Connect the DS18B20 sensor and the WS2812B LED strip to the ESP32 as per their respective datasheets.
2. Update the "secrets.h" file with your WiFi and ThingSpeak information.
3. Update the relevant pin numbers in the code where the sensor and LED strip are connected.
4. Upload the Arduino sketch to the ESP32.

## Note

The ESP32's deep sleep mode is used to save power between temperature readings. Therefore, this project is suitable for running on battery power. Make sure to disconnect from WiFi before the ESP32 goes into deep sleep to conserve power.

## Contributions

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is open-sourced under the MIT license.