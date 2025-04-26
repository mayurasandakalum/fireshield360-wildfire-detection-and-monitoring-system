#include <Arduino.h>
#include "smoke.h"

// Global variables
static int smokeSensorPin = -1;                  // Pin connected to MQ2 sensor
static int lastSmokeValue = 0;                   // Last read smoke value
static bool smokeReadingValid = false;           // Flag for valid reading
static unsigned long lastReadTime = 0;           // Time of last reading
static const unsigned long READ_INTERVAL = 1000; // Read every 1 second
static int smokeThreshold = 1500;                // Default threshold for smoke detection

// Constants for validation
static const int SMOKE_MIN_VALID = 100;    // Minimum valid reading
static const int SMOKE_MAX_VALID = 4000;   // Maximum valid reading
static const int CONSECUTIVE_READINGS = 3; // Number of readings to check for stability
static int previousReadings[3] = {0, 0, 0};
static int readingIndex = 0;

void initSmokeSensor(int pin)
{
    smokeSensorPin = pin;

    // MQ2 requires analog input
    pinMode(smokeSensorPin, INPUT);

    // Initialize previous readings array
    for (int i = 0; i < CONSECUTIVE_READINGS; i++)
    {
        previousReadings[i] = analogRead(smokeSensorPin);
        delay(10); // Small delay between readings
    }

    // Initial reading
    lastSmokeValue = analogRead(smokeSensorPin);

    // Check if the reading is within valid range
    smokeReadingValid = (lastSmokeValue >= SMOKE_MIN_VALID &&
                         lastSmokeValue <= SMOKE_MAX_VALID);

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
        int currentReading = analogRead(smokeSensorPin);

        // Store current reading in the array for stability check
        previousReadings[readingIndex] = currentReading;
        readingIndex = (readingIndex + 1) % CONSECUTIVE_READINGS;

        lastSmokeValue = currentReading;

        // Check if reading is within valid range
        smokeReadingValid = (currentReading >= SMOKE_MIN_VALID &&
                             currentReading <= SMOKE_MAX_VALID);

        // Additional check for reading stability (optional)
        if (smokeReadingValid)
        {
            // Check if readings are too stable (might indicate disconnection)
            bool allReadingsSame = true;
            for (int i = 1; i < CONSECUTIVE_READINGS; i++)
            {
                if (previousReadings[0] != previousReadings[i])
                {
                    allReadingsSame = false;
                    break;
                }
            }

            // If all readings are identical, sensor might be disconnected
            if (allReadingsSame && currentReading > 0)
            {
                smokeReadingValid = false;
            }
        }

        lastReadTime = currentTime;

        // // Debug output
        // Serial.print("Smoke sensor reading: ");
        // Serial.print(lastSmokeValue);
        // Serial.print(" Valid: ");
        // Serial.println(smokeReadingValid ? "Yes" : "No");
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

int getLocalSmokeThreshold()
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
