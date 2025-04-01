#ifndef SMOKE_H
#define SMOKE_H

// Initialize smoke sensor (MQ2)
void initSmokeSensor(int pin);

// Read smoke value from the sensor
int readSmokeValue();

// Get last read smoke value
int getSmokeValue();

// Check if the smoke reading is valid
bool isSmokeReadingValid();

// Get smoke detection threshold
int getSmokeThreshold();

// Set smoke detection threshold
void setSmokeThreshold(int threshold);

// Check if smoke is detected (above threshold)
bool isSmokeDetected();

#endif // SMOKE_H
