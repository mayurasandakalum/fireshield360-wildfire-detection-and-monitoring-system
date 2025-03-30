#include <Arduino.h>
#include "led.h"

// The pin number for the built-in LED
const int LED_PIN = 2; // GPIO2 is the built-in LED on most ESP32 dev boards

void initLed()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void turnOnLed()
{
    digitalWrite(LED_PIN, HIGH);
}

void turnOffLed()
{
    digitalWrite(LED_PIN, LOW);
}

void blinkLed(int delayMs)
{
    turnOnLed();
    delay(delayMs);
    turnOffLed();
    delay(delayMs);
}
