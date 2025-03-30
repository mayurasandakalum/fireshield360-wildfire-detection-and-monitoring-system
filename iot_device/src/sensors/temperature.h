#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <DHTesp.h>

// Initialize temperature sensor
void initTemperatureSensor(int pin);

// Read temperature value from the sensor
float readTemperature();

// Get last read temperature value
float getTemperature();

// Check if the temperature reading is valid
bool isTemperatureReadingValid();

// Get DHT instance (useful for direct access if needed)
DHTesp *getTemperatureSensorInstance();

#endif // TEMPERATURE_H
