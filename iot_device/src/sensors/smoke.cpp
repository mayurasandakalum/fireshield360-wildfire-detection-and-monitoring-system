#include <Arduino.h>
#include "smoke.h"

// Global variables
static int smokeSensorPin = -1;                  // Pin connected to MQ2 sensor
static int lastSmokeValue = 0;                   // Last read smoke value
static bool smokeReadingValid = false;           // Flag for valid reading
static unsigned long lastReadTime = 0;           // Time of last reading
static const unsigned long READ_INTERVAL = 1000; // Read every 1 second
static int smokeThreshold = 1500;                // Default threshold for smoke detection

void initSmokeSensor(int pin)
{
    smokeSensorPin = pin;

    // MQ2 requires analog input
    pinMode(smokeSensorPin, INPUT);

    // Initial reading
    lastSmokeValue = analogRead(smokeSensorPin);
    smokeReadingValid = true; // Analog readings are generally always valid
    lastReadTime = millis();

    Serial.print("MQ2 Smoke Sensor initialized on pin ");
    Serial.println(smokeSensorPin);
}

int readSmokeValue()
{
    // Check if minimum interval has passed since last read
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastSmokeValue = analogRead(smokeSensorPin);
        smokeReadingValid = true;
        lastReadTime = currentTime;

        // Debug output
        Serial.print("Smoke sensor reading: ");
        Serial.println(lastSmokeValue);
    }
    return lastSmokeValue;
}

int getSmokeValue()
{
    return lastSmokeValue;
}

bool isSmokeReadingValid()
{
    return smokeReadingValid;
}

int getSmokeThreshold()
{
    return smokeThreshold;
}

void setSmokeThreshold(int threshold)
{
    smokeThreshold = threshold;
}

bool isSmokeDetected()
{
    // Higher values indicate more gas detected
    return (lastSmokeValue > smokeThreshold);
}
