#ifndef LED_H
#define LED_H

// Function to initialize an LED pin
void initLed(int pin);

// Function to turn on an LED
void turnOnLed(int pin);

// Function to turn off an LED
void turnOffLed(int pin);

// Function to blink an LED with the specified delay in milliseconds
void blinkLed(int pin, int delayMs);

#endif // LED_H
