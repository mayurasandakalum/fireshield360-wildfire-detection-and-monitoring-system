#ifndef INFRARED_H
#define INFRARED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

// Initialize the infrared temperature sensor
void initInfraredSensor();

// Read the ambient temperature from the infrared sensor
float readAmbientTemperature();

// Read the object temperature from the infrared sensor
float readObjectTemperature();

// Check if the infrared sensor reading is valid
bool isInfraredReadingValid();

// Get the infrared sensor instance
Adafruit_MLX90614 *getInfraredSensorInstance();

#endif // INFRARED_H
