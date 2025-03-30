#include <Arduino.h>
#include "temperature.h"

// Global variables
static DHTesp dht;                               // Added static to limit scope to this file
static float lastTemperature = 0.0;              // Added static
static bool temperatureValid = false;            // Added static
static unsigned long lastReadTime = 0;           // Added static
static const unsigned long READ_INTERVAL = 2000; // Added static

void initTemperatureSensor(int pin)
{
    dht.setup(pin, DHTesp::DHT11); // Initialize DHT sensor as DHT11
    // Perform initial reading
    lastTemperature = dht.getTemperature();
    temperatureValid = !isnan(lastTemperature);
    lastReadTime = millis();
}

float readTemperature()
{
    // Check if minimum interval has passed since last read
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastTemperature = dht.getTemperature();
        temperatureValid = !isnan(lastTemperature);
        lastReadTime = currentTime;
    }
    return lastTemperature;
}

float getTemperature()
{
    return lastTemperature;
}

bool isTemperatureReadingValid()
{
    return temperatureValid;
}

DHTesp *getTemperatureSensorInstance()
{
    return &dht;
}
