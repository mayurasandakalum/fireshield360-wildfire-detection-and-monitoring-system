#ifndef HUMIDITY_H
#define HUMIDITY_H

#include <DHTesp.h>

// Initialize humidity sensor
void initHumiditySensor(int pin);

// Read humidity value from the sensor
float readHumidity();

// Get last read humidity value
float getHumidity();

// Check if the humidity reading is valid
bool isHumidityReadingValid();

// Get DHT instance (useful for direct access if needed)
DHTesp *getHumiditySensorInstance();

#endif // HUMIDITY_H
