#include <Arduino.h>
#include "led.h"

// The pin number for the built-in LED
const int LED_PIN = 2; // GPIO2 is the built-in LED on most ESP32 dev boards

void initLed(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void turnOnLed(int pin)
{
    digitalWrite(pin, HIGH);
}

void turnOffLed(int pin)
{
    digitalWrite(pin, LOW);
}

void blinkLed(int pin, int delayMs)
{
    turnOnLed(pin);
    delay(delayMs);
    turnOffLed(pin);
    delay(delayMs);
}
