#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h> 

float readTemperatureSensor();
void identifySensor(byte addr[]);

#endif
