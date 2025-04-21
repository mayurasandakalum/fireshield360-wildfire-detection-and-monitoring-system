#include <Arduino.h>
#include "wildfire_detection.h"

// Default threshold values
static float temperatureThreshold = 30.0;   // in Celsius, temperatures above this are dangerous
static float humidityThreshold = 30;        // in percent, humidity below this is dangerous
static int smokeThreshold = 1500;           // Raw sensor value, above this indicates smoke
static float irTemperatureThreshold = 40.0; // in Celsius, IR temperatures above this are dangerous

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
    Serial.print("IR Temperature: > ");
    Serial.print(irTemperatureThreshold);
    Serial.println(" °C");
}

void setWildfireThresholds(float tempThreshold, float humidityThreshold, int smokeThreshold, float irTempThreshold)
{
    temperatureThreshold = tempThreshold;
    humidityThreshold = humidityThreshold;
    smokeThreshold = smokeThreshold;
    irTemperatureThreshold = irTempThreshold;

    Serial.println("Updated wildfire detection thresholds:");
    Serial.print("Temperature: > ");
    Serial.print(temperatureThreshold);
    Serial.println(" °C");
    Serial.print("Humidity: < ");
    Serial.print(humidityThreshold);
    Serial.println(" %");
    Serial.print("Smoke: > ");
    Serial.println(smokeThreshold);
    Serial.print("IR Temperature: > ");
    Serial.print(irTemperatureThreshold);
    Serial.println(" °C");
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

float getIRTemperatureThreshold()
{
    return irTemperatureThreshold;
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

bool isIRTemperatureExceedingThreshold(float irTemperature)
{
    return irTemperature > irTemperatureThreshold;
}

int getNumberOfThresholdsExceeded(float temperature, float humidity, int smokeValue, float irTemperature)
{
    int count = 0;

    if (isTemperatureExceedingThreshold(temperature))
        count++;
    if (isHumidityBelowThreshold(humidity))
        count++;
    if (isSmokeExceedingThreshold(smokeValue))
        count++;
    if (isIRTemperatureExceedingThreshold(irTemperature))
        count++;

    return count;
}

bool isWildfireDetected(float temperature, float humidity, int smokeValue, float irTemperature)
{
    // Wildfire is detected when at least 2 sensors exceed thresholds
    return getNumberOfThresholdsExceeded(temperature, humidity, smokeValue, irTemperature) >= 2;
}
