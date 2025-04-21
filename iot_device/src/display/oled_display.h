#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED pins
#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_DC 16
#define OLED_CS 5
#define OLED_RESET 17

// External declaration of the display object
extern Adafruit_SSD1306 display;

// External declaration of the logo
extern const unsigned char electronicshub_logo[];

// External declaration of the warning icons and fire animation frames
extern const unsigned char warning_icon[];
extern const unsigned char large_warning_icon[];
extern const unsigned char fire_anim_1[];
extern const unsigned char fire_anim_2[];
extern const unsigned char fire_anim_3[];

// Function to initialize the OLED display
bool initDisplay();

// Demo and utility functions
void allPixels();
void textDisplay(const String &text = "Electronics Hub");
void invertedTextDisplay(const String &text = "Electronics Hub");
void scrollText();
void displayChars();
void textSize();
void drawRectangle();
void drawFilledRectangle();
void drawRoundRectangle();
void drawFilledRoundRectangle();
void drawCircle();
void drawFilledCircle();
void drawTriangle();
void drawFilledTriangle();

// Function to display sensor data (updated to include IR temperature)
void displaySensorData(float temperature, float humidity, int smokeValue, float irTemperature, bool wildfireDetected = false);

// Function to display a splash screen with logo
void displaySplashScreen(uint16_t displayTime = 2000);

// Function to clear the display
void clearDisplay();

// Include wildfire detection for threshold checks
#include "../detection/wildfire_detection.h"

#endif // OLED_DISPLAY_H
