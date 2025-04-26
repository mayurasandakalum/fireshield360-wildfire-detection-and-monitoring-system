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

// RGB LED Functions
void initRgbLed(int redPin, int greenPin, int bluePin)
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    // Initialize with all LEDs off (common anode configuration)
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
}

void setRgbLedColor(int redPin, int greenPin, int bluePin, int redValue, int greenValue, int blueValue)
{
    digitalWrite(redPin, redValue > 0 ? HIGH : LOW);
    digitalWrite(greenPin, greenValue > 0 ? HIGH : LOW);
    digitalWrite(bluePin, blueValue > 0 ? HIGH : LOW);
}

void setRgbLedRed(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 1, 0, 0);
}

void setRgbLedGreen(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 0, 1, 0);
}

void setRgbLedBlue(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 0, 0, 1);
}

void setRgbLedYellow(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 1, 1, 0);
}

void setRgbLedPurple(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 1, 0, 1);
}

void setRgbLedCyan(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 0, 1, 1);
}

void setRgbLedWhite(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 1, 1, 1);
}

void turnOffRgbLed(int redPin, int greenPin, int bluePin)
{
    setRgbLedColor(redPin, greenPin, bluePin, 0, 0, 0);
}

void blinkRgbLed(int redPin, int greenPin, int bluePin,
                 int redValue, int greenValue, int blueValue, int delayMs)
{
    setRgbLedColor(redPin, greenPin, bluePin, redValue, greenValue, blueValue);
    delay(delayMs);
    turnOffRgbLed(redPin, greenPin, bluePin);
    delay(delayMs);
}

// Sensor status indicator functions
void setSensorNormalStatus(int redPin, int greenPin, int bluePin)
{
    // Normal Operation: Green
    setRgbLedGreen(redPin, greenPin, bluePin);
}

void setSensorErrorStatus(int redPin, int greenPin, int bluePin)
{
    // Sensor Error: Red
    setRgbLedRed(redPin, greenPin, bluePin);
}

void setSensorInitializationStatus(int redPin, int greenPin, int bluePin)
{
    // Power/Initialization: Blue
    setRgbLedBlue(redPin, greenPin, bluePin);
}

void showSensorInitializationSequence(int redPin, int greenPin, int bluePin, int numBlinks, int blinkDelay)
{
    // Show blue light during initialization/power-up
    Serial.println("Power/Initialization: Setting RGB LED to BLUE");

    // Blink blue light to indicate system is initializing
    for (int i = 0; i < numBlinks; i++)
    {
        setSensorInitializationStatus(redPin, greenPin, bluePin);
        delay(blinkDelay);
        turnOffRgbLed(redPin, greenPin, bluePin);
        delay(blinkDelay);
    }

    // End with solid blue to indicate ready for operation
    setSensorInitializationStatus(redPin, greenPin, bluePin);
    delay(500); // Keep blue LED on for a moment
}

bool updateSensorStatusLed(int redPin, int greenPin, int bluePin, bool isValid)
{
    if (!isValid)
    {
        // Sensor error - show red
        setSensorErrorStatus(redPin, greenPin, bluePin);
        return false;
    }
    else
    {
        // Normal operation - show green
        setSensorNormalStatus(redPin, greenPin, bluePin);
        return true;
    }
}
