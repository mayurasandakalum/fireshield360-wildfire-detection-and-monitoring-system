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

// RGB LED Functions
// Function to initialize RGB LED pins
void initRgbLed(int redPin, int greenPin, int bluePin);

// Functions to set RGB LED to specific colors
void setRgbLedColor(int redPin, int greenPin, int bluePin, int redValue, int greenValue, int blueValue);
void setRgbLedRed(int redPin, int greenPin, int bluePin);
void setRgbLedGreen(int redPin, int greenPin, int bluePin);
void setRgbLedBlue(int redPin, int greenPin, int bluePin);
void setRgbLedYellow(int redPin, int greenPin, int bluePin);
void setRgbLedPurple(int redPin, int greenPin, int bluePin);
void setRgbLedCyan(int redPin, int greenPin, int bluePin);
void setRgbLedWhite(int redPin, int greenPin, int bluePin);
void turnOffRgbLed(int redPin, int greenPin, int bluePin);

// Function to blink RGB LED with specified color and delay
void blinkRgbLed(int redPin, int greenPin, int bluePin,
                 int redValue, int greenValue, int blueValue, int delayMs);

// Sensor status indicator functions using the standardized color scheme
void setSensorNormalStatus(int redPin, int greenPin, int bluePin);         // Green
void setSensorErrorStatus(int redPin, int greenPin, int bluePin);          // Red
void setSensorInitializationStatus(int redPin, int greenPin, int bluePin); // Blue

// Function to show initialization sequence
void showSensorInitializationSequence(int redPin, int greenPin, int bluePin, int numBlinks = 5, int blinkDelay = 200);

// Check sensors and update RGB LED status
bool updateSensorStatusLed(int redPin, int greenPin, int bluePin, bool isValid);

#endif // LED_H
