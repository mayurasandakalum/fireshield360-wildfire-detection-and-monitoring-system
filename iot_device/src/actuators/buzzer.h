#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

// Initialize buzzer
void initBuzzer(int pin);

// Sound fire alert pattern for specified duration (in milliseconds)
void soundFireAlert(unsigned long duration);

// Sound verified alert with 1kHz tone for specified duration (in milliseconds)
void soundVerifiedAlert(unsigned long duration);

// Stop buzzer immediately
void stopBuzzer();

// Update buzzer state (call this in the main loop)
void updateBuzzer();

// Check if buzzer is currently active
bool isBuzzerActive();

#endif // BUZZER_H
