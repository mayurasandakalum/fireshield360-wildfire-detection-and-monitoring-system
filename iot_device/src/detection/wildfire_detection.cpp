#include <Arduino.h>
#include "wildfire_detection.h"

// Default threshold values
static float temperatureThreshold = 40.0; // in Celsius, temperatures above this are dangerous
static float humidityThreshold = 30;    // in percent, humidity below this is dangerous
static int smokeThreshold = 1500;         // Raw sensor value, above this indicates smoke

void initWildfireDetection()
{
    Serial.println("Wildfire detection initialized with thresholds:");
    Serial.print("Temperature: > ");
    Serial.print(temperatureThreshold);
    Serial.println(" °C");
    Serial.print("Humidity: < ");
    Serial.print(humidityThreshold);
    Serial.println(" %");
    Serial.print("Smoke: > ");
    Serial.println(smokeThreshold);
}

void setWildfireThresholds(float tempThreshold, float humidityThreshold, int smokeThreshold)
{
    temperatureThreshold = tempThreshold;
    humidityThreshold = humidityThreshold;
    smokeThreshold = smokeThreshold;

    Serial.println("Updated wildfire detection thresholds:");
    Serial.print("Temperature: > ");
    Serial.print(temperatureThreshold);
    Serial.println(" °C");
    Serial.print("Humidity: < ");
    Serial.print(humidityThreshold);
    Serial.println(" %");
    Serial.print("Smoke: > ");
    Serial.println(smokeThreshold);
}

float getTemperatureThreshold()
{
    return temperatureThreshold;
}

float getHumidityThreshold()
{
    return humidityThreshold;
}

int getSmokeThreshold()
{
    return smokeThreshold;
}

bool isTemperatureExceedingThreshold(float temperature)
{
    return temperature > temperatureThreshold;
}

bool isHumidityBelowThreshold(float humidity)
{
    return humidity < humidityThreshold;
}

bool isSmokeExceedingThreshold(int smokeValue)
{
    return smokeValue > smokeThreshold;
}

int getNumberOfThresholdsExceeded(float temperature, float humidity, int smokeValue)
{
    int count = 0;

    if (isTemperatureExceedingThreshold(temperature))
        count++;
    if (isHumidityBelowThreshold(humidity))
        count++;
    if (isSmokeExceedingThreshold(smokeValue))
        count++;

    return count;
}

bool isWildfireDetected(float temperature, float humidity, int smokeValue)
{
    // Wildfire is detected when at least 2 sensors exceed thresholds
    return getNumberOfThresholdsExceeded(temperature, humidity, smokeValue) >= 2;
}
