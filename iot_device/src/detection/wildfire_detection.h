#ifndef WILDFIRE_DETECTION_H
#define WILDFIRE_DETECTION_H

// Initialize wildfire detection with default thresholds
void initWildfireDetection();

// Set thresholds for wildfire detection
void setWildfireThresholds(float tempThreshold, float humidityThreshold, int smokeThreshold, float irTempThreshold);

// Get current thresholds
float getTemperatureThreshold();
float getHumidityThreshold();
int getSmokeThreshold();
float getIRTemperatureThreshold();

// Check if individual sensors exceed thresholds
bool isTemperatureExceedingThreshold(float temperature);
bool isHumidityBelowThreshold(float humidity); // Note: For humidity, low values are dangerous
bool isSmokeExceedingThreshold(int smokeValue);
bool isIRTemperatureExceedingThreshold(float irTemperature);

// Check if wildfire is detected (at least 2 sensors exceed thresholds)
bool isWildfireDetected(float temperature, float humidity, int smokeValue, float irTemperature);

// Get number of thresholds exceeded
int getNumberOfThresholdsExceeded(float temperature, float humidity, int smokeValue, float irTemperature);

#endif // WILDFIRE_DETECTION_H
