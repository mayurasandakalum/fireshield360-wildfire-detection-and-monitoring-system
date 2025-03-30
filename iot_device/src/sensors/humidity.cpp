#include <Arduino.h>
#include "humidity.h"
#include "temperature.h"

// Global variables
static DHTesp *dht;                              // Added static to limit scope to this file
static float lastHumidity = 0.0;                 // Added static
static bool humidityValid = false;               // Added static
static unsigned long lastReadTime = 0;           // Added static
static const unsigned long READ_INTERVAL = 2000; // Added static

void initHumiditySensor(int pin)
{
    // The DHT sensor is already initialized in temperature module,
    // so we just point to it from the temperature module
    dht = getTemperatureSensorInstance();
    // Perform initial reading
    lastHumidity = dht->getHumidity();
    humidityValid = !isnan(lastHumidity);
    lastReadTime = millis();
}

float readHumidity()
{
    // Check if minimum interval has passed since last read
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastHumidity = dht->getHumidity();
        humidityValid = !isnan(lastHumidity);
        lastReadTime = currentTime;
    }
    return lastHumidity;
}

float getHumidity()
{
    return lastHumidity;
}

bool isHumidityReadingValid()
{
    return humidityValid;
}

DHTesp *getHumiditySensorInstance()
{
    return dht;
}
