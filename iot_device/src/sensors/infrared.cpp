#include "infrared.h"

// Global variables
static Adafruit_MLX90614 mlx;                    // MLX90614 sensor instance
static float lastObjectTemp = 0.0;               // Last object temperature reading
static float lastAmbientTemp = 0.0;              // Last ambient temperature reading
static bool infraredValid = false;               // Validity flag
static unsigned long lastReadTime = 0;           // Time of last reading
static const unsigned long READ_INTERVAL = 1000; // Read interval in ms

void initInfraredSensor()
{
    // Initialize I2C with default pins (21, 22)
    // Wire is already initialized in main.cpp

    // Initialize the MLX90614 sensor
    if (mlx.begin())
    {
        Serial.println("MLX90614 infrared sensor initialized successfully");
        // Take initial readings
        lastAmbientTemp = mlx.readAmbientTempC();
        lastObjectTemp = mlx.readObjectTempC();
        infraredValid = !isnan(lastObjectTemp) && !isnan(lastAmbientTemp);
    }
    else
    {
        Serial.println("ERROR: Could not initialize MLX90614 infrared sensor");
        infraredValid = false;
    }
    lastReadTime = millis();
}

float readAmbientTemperature()
{
    // Check if minimum interval has passed since last read
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastAmbientTemp = mlx.readAmbientTempC();
        infraredValid = !isnan(lastAmbientTemp) && !isnan(lastObjectTemp);
        lastReadTime = currentTime;
    }
    return lastAmbientTemp;
}

float readObjectTemperature()
{
    // Check if minimum interval has passed since last read
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastObjectTemp = mlx.readObjectTempC();
        infraredValid = !isnan(lastObjectTemp) && !isnan(lastAmbientTemp);
        lastReadTime = currentTime;
    }
    return lastObjectTemp;
}

bool isInfraredReadingValid()
{
    return infraredValid;
}

Adafruit_MLX90614 *getInfraredSensorInstance()
{
    return &mlx;
}
