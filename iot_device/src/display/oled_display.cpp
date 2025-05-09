#include "oled_display.h"

// Initialize the display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// The logo bitmap
const unsigned char fireshield360_logo[] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x07, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0x80, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x07, 0xe0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x07, 0xe0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x0f, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x1f, 0xc4, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xc4, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x7f, 0xcc, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x7f, 0xce, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xfc, 0x7f, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xfc, 0x7f, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xf8, 0x2f, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xf8, 0x0f, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xf0, 0x0f, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xe0, 0x07, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0xe0, 0x0f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x60, 0x0e, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x30, 0x0c, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x10, 0x10, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// Warning icon bitmap (16x16 pixels, triangle with exclamation mark)
const unsigned char warning_icon[] PROGMEM = {
    0x00, 0x00, // 0000000000000000
    0x00, 0x00, // 0000000000000000
    0x01, 0x00, // 0000000100000000
    0x03, 0x80, // 0000001110000000
    0x03, 0x80, // 0000001110000000
    0x07, 0xC0, // 0000011111000000
    0x07, 0xC0, // 0000011111000000
    0x0F, 0xE0, // 0000111111100000
    0x0F, 0xE0, // 0000111111100000
    0x1F, 0xF0, // 0001111111110000
    0x1E, 0x70, // 0001111001110000
    0x3E, 0x78, // 0011111001111000
    0x3F, 0xF8, // 0011111111111000
    0x7F, 0xFC, // 0111111111111100
    0xFF, 0xFE, // 1111111111111110
    0x00, 0x00  // 0000000000000000
};

// Fire animation frames (32x32 pixels each, 3 frames)
const unsigned char fire_anim_1[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00,
    0x00, 0x0F, 0xF0, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x7F, 0xFE, 0x00,
    0x00, 0xFF, 0xFF, 0x00, 0x01, 0xFA, 0x5F, 0x80, 0x03, 0xF0, 0x0F, 0xC0, 0x07, 0xE0, 0x07, 0xE0,
    0x0F, 0xC1, 0x83, 0xF0, 0x1F, 0x83, 0xC1, 0xF8, 0x3F, 0x07, 0xE0, 0xFC, 0x7E, 0x0F, 0xF0, 0x7E,
    0xFC, 0x1F, 0xF8, 0x3F, 0xF8, 0x3F, 0xFC, 0x1F, 0xF8, 0x7F, 0xFE, 0x1F, 0xF0, 0xFF, 0xFF, 0x0F,
    0xF1, 0xFF, 0xFF, 0x8F, 0xF3, 0xFF, 0xFF, 0xCF, 0xF7, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFC,
    0x1F, 0xFF, 0xFF, 0xF8, 0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x01, 0xFF, 0xFF, 0x80};

const unsigned char fire_anim_2[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x07, 0xE0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x3F, 0xFC, 0x00,
    0x00, 0x7F, 0xFE, 0x00, 0x00, 0xFE, 0x7F, 0x00, 0x01, 0xFC, 0x3F, 0x80, 0x03, 0xF8, 0x1F, 0xC0,
    0x07, 0xF0, 0x0F, 0xE0, 0x0F, 0xE2, 0x47, 0xF0, 0x1F, 0xC6, 0x63, 0xF8, 0x3F, 0x8E, 0x71, 0xFC,
    0x7F, 0x1E, 0x78, 0xFE, 0x7E, 0x3F, 0xFC, 0x7E, 0xFC, 0x7F, 0xFE, 0x3F, 0xF8, 0xFF, 0xFF, 0x1F,
    0xF1, 0xFF, 0xFF, 0x8F, 0xF3, 0xFF, 0xFF, 0xCF, 0xF7, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFC, 0x1F, 0xFF, 0xFF, 0xF8,
    0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x03, 0xFF, 0xFF, 0xC0, 0x00, 0xFF, 0xFF, 0x00};

const unsigned char fire_anim_3[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00,
    0x00, 0x0F, 0xF0, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x7F, 0xFE, 0x00,
    0x00, 0xFF, 0xFF, 0x00, 0x01, 0xFE, 0x7F, 0x80, 0x03, 0xFC, 0x3F, 0xC0, 0x07, 0xF0, 0x0F, 0xE0,
    0x0F, 0xE0, 0x07, 0xF0, 0x1F, 0xC0, 0x03, 0xF8, 0x3F, 0x80, 0x01, 0xFC, 0x7F, 0x00, 0x00, 0xFE,
    0xFE, 0x10, 0x08, 0x7F, 0xFC, 0x38, 0x1C, 0x3F, 0xF8, 0x7C, 0x3E, 0x1F, 0xF0, 0xFF, 0xFF, 0x0F,
    0xF1, 0xFF, 0xFF, 0x8F, 0xF3, 0xFF, 0xFF, 0xCF, 0xF7, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x7F, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFC, 0x1F, 0xFF, 0xFF, 0xF8, 0x0F, 0xFF, 0xFF, 0xF0,
    0x07, 0xFF, 0xFF, 0xE0, 0x03, 0xFF, 0xFF, 0xC0, 0x01, 0xFF, 0xFF, 0x80, 0x00, 0x7F, 0xFE, 0x00};

// Large warning triangle icon for alert
const unsigned char large_warning_icon[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x03, 0xE0, 0x00,
    0x00, 0x07, 0xF0, 0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0x3F, 0xFE, 0x00,
    0x00, 0x7F, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0xC0, 0x03, 0xFF, 0xFF, 0xE0,
    0x07, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0x1F, 0xF8, 0x1F, 0xFE, 0x0F, 0xFC, 0x3F, 0xFC, 0x07, 0xFE,
    0x7F, 0xF8, 0x03, 0xFF, 0x7F, 0xF0, 0x01, 0xFF, 0xFF, 0xE0, 0x00, 0xFF, 0xFF, 0xE0, 0x00, 0xFF,
    0xFF, 0xE0, 0x00, 0xFF, 0xFF, 0xF0, 0x01, 0xFF, 0x7F, 0xF8, 0x03, 0xFF, 0x3F, 0xFC, 0x07, 0xFE,
    0x1F, 0xFE, 0x0F, 0xFC, 0x0F, 0xFF, 0x1F, 0xF8, 0x07, 0xFF, 0xFF, 0xF0, 0x03, 0xFF, 0xFF, 0xE0,
    0x01, 0xFF, 0xFF, 0xC0, 0x00, 0xFF, 0xFF, 0x80, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x3F, 0xFE, 0x00};

// Add a power icon bitmap (32x32 pixels)
const unsigned char power_icon[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xC0, 0x00,
    0x00, 0x07, 0xE0, 0x00,
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x3F, 0xFC, 0x00,
    0x00, 0x7F, 0xFE, 0x00,
    0x00, 0x7F, 0xFE, 0x00,
    0x00, 0x7C, 0x3E, 0x00,
    0x00, 0xF8, 0x1F, 0x00,
    0x00, 0xF8, 0x1F, 0x00,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x01, 0xF0, 0x0F, 0x80,
    0x00, 0xF8, 0x1F, 0x00,
    0x00, 0xF8, 0x1F, 0x00,
    0x00, 0x7C, 0x3E, 0x00,
    0x00, 0x7F, 0xFE, 0x00,
    0x00, 0x7F, 0xFE, 0x00,
    0x00, 0x3F, 0xFC, 0x00,
    0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x07, 0xE0, 0x00,
    0x00, 0x03, 0xC0, 0x00,
    0x00, 0x00, 0x00, 0x00};

bool initDisplay()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC))
    {
        Serial.println(F("SSD1306 allocation failed"));
        return false;
    }

    display.clearDisplay();
    display.display();
    return true;
}

void displaySplashScreen(uint16_t displayTime)
{
    display.clearDisplay();
    display.drawBitmap(0, 0, fireshield360_logo, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.display();
    delay(displayTime);
}

void clearDisplay()
{
    display.clearDisplay();
    display.display();
}

void allPixels()
{
    int i;
    int j;
    display.clearDisplay();
    for (i = 0; i < 64; i++)
    {
        for (j = 0; j < 128; j++)
        {
            display.drawPixel(j, i, SSD1306_WHITE);
        }
        display.display();
        delay(30);
    }

    for (i = 0; i < 64; i++)
    {
        for (j = 0; j < 128; j++)
        {
            display.drawPixel(j, i, SSD1306_BLACK);
        }
        display.display();
        delay(30);
    }
}

void textDisplay(const String &text)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    // Center the text horizontally
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (SCREEN_WIDTH - w) / 2;
    int16_t y = 28; // keep vertical position as before
    display.setCursor(x, y);
    display.println(text);
    display.display();
    delay(3000);
}

void invertedTextDisplay(const String &text)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setCursor(5, 28);
    display.println(text);
    display.display();
    delay(3000);
}

void scrollText()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("This is a");
    display.println("Scrolling");
    display.println("Text!");
    display.display();
    delay(100);
    display.startscrollright(0x00, 0x0F);
    delay(2000);
    display.startscrollleft(0x00, 0x0F);
    delay(2000);
    display.startscrolldiagright(0x00, 0x0F);
    delay(2000);
    display.startscrolldiagleft(0x00, 0x0F);
    delay(2000);
    display.stopscroll();
}

void displayChars()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.cp437(true);

    for (int16_t i = 0; i < 256; i++)
    {
        if (i == '\n')
        {
            display.write(' ');
        }
        else
        {
            display.write(i);
        }
    }

    display.display();
    delay(4000);
}

void textSize()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Size: 1"));
    display.println(F("ABC"));

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.println("Size: 2");
    display.println(F("ABC"));

    display.display();
    delay(3000);
}

void drawRectangle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Rectangle");
    display.drawRect(0, 15, 90, 45, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawFilledRectangle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Filled Rectangle");
    display.fillRect(0, 15, 90, 45, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawRoundRectangle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Round Rectangle");
    display.drawRoundRect(0, 15, 90, 45, 10, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawFilledRoundRectangle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Filled Round Rect");
    display.fillRoundRect(0, 15, 90, 45, 10, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawCircle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Circle");
    display.drawCircle(30, 36, 25, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawFilledCircle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Filled Circle");
    display.fillCircle(30, 36, 25, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawTriangle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Triangle");
    display.drawTriangle(30, 15, 0, 60, 60, 60, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void drawFilledTriangle()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Filled Triangle");
    display.fillTriangle(30, 15, 0, 60, 60, 60, SSD1306_WHITE);
    display.display();
    delay(2000);
}

void displaySensorData(float temperature, float humidity, int smokeValue)
{
    display.clearDisplay();

    // Set text properties
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Display title
    display.setCursor(0, 0);
    display.println("Sensor Readings:");

    // Display temperature
    display.setCursor(0, 16);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    // Display humidity
    display.setCursor(0, 26);
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");

    // Display smoke value
    display.setCursor(0, 36);
    display.print("Smoke: ");
    display.println(smokeValue);

    // Display time since last update
    display.setCursor(0, 46);
    display.print("Last update: ");
    display.print(millis() / 1000);
    display.println("s");

    // Update the display
    display.display();
}

void displaySensorData(float temperature, float humidity, int smokeValue, bool wildfireDetected)
{
    display.clearDisplay();

    // Define the display bands
    const int YELLOW_BAND_HEIGHT = 16; // 25% of 64 pixels (2 rows of text)

    // Draw the yellow band on top (for title area)
    display.fillRect(0, 0, SCREEN_WIDTH, YELLOW_BAND_HEIGHT, SSD1306_WHITE);

    // Set text properties for title in yellow area (black text on yellow background)
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);

    if (wildfireDetected)
    {
        // Animate the warning text in the yellow band
        int animPhase = (millis() / 200) % 4; // 4 animation phases, changing every 200ms

        switch (animPhase)
        {
        case 0:
            display.setCursor(4, 4);
            display.print("! WILDFIRE ALERT !");
            break;
        case 1:
            display.setCursor(4, 4);
            display.print(" WILDFIRE ALERT  ");
            break;
        case 2:
            display.setCursor(4, 4);
            display.print("  WILDFIRE ALERT ");
            break;
        case 3:
            display.setCursor(4, 4);
            display.print(" ! DANGER ! ");
            break;
        }

        // Draw small warning triangle on the right side
        display.drawBitmap(110, 0, warning_icon, 16, 16, SSD1306_BLACK);
    }
    else
    {
        display.setCursor(0, 4); // Center vertically in yellow band
        display.println(" Sensor Readings ");
    }

    // Switch to white text on black for the blue area
    display.setTextColor(SSD1306_WHITE);

    // Start a bit closer to the yellow band to save space
    int yPos = YELLOW_BAND_HEIGHT + 1;

    // Display temperature with threshold indicator
    display.setCursor(0, yPos);
    display.print("Temp: ");
    display.print(temperature);
    display.print(" C");
    if (isTemperatureExceedingThreshold(temperature))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display humidity with threshold indicator
    display.setCursor(0, yPos);
    display.print("Humidity: ");
    display.print(humidity);
    display.print(" %");
    if (isHumidityBelowThreshold(humidity))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display smoke value with threshold indicator
    display.setCursor(0, yPos);
    display.print("Smoke: ");
    display.print(smokeValue);
    if (isSmokeExceedingThreshold(smokeValue))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display thresholds exceeded - Fix: Pass 0.0 as default IR temperature
    display.setCursor(0, yPos);
    int thresholdsExceeded = getNumberOfThresholdsExceeded(temperature, humidity, smokeValue, 0.0);
    display.print("Alerts: ");
    display.print(thresholdsExceeded);
    display.print("/4"); // Update to reflect 4 possible alerts now
    yPos += 9;           // Reduce spacing from 10 to 9

    // Display time since last update - ensure it's fully visible
    display.setCursor(0, yPos);
    display.print("Update: "); // Shortened from "Last update: " to save space
    display.print(millis() / 1000);
    display.print("s"); // Removed println to avoid extra line feed

    // Update the display
    display.display();
}

// Updated to include IR temperature
void displaySensorData(float temperature, float humidity, int smokeValue, float irTemperature, bool wildfireDetected)
{
    display.clearDisplay();

    // Define the display bands
    const int YELLOW_BAND_HEIGHT = 16; // 25% of 64 pixels (2 rows of text)

    // Draw the yellow band on top (for title area)
    display.fillRect(0, 0, SCREEN_WIDTH, YELLOW_BAND_HEIGHT, SSD1306_WHITE);

    // Set text properties for title in yellow area (black text on yellow background)
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);

    if (wildfireDetected)
    {
        // Animate the warning text in the yellow band
        int animPhase = (millis() / 200) % 4; // 4 animation phases, changing every 200ms

        switch (animPhase)
        {
        case 0:
            display.setCursor(4, 4);
            display.print("! WILDFIRE ALERT !");
            break;
        case 1:
            display.setCursor(4, 4);
            display.print(" WILDFIRE ALERT  ");
            break;
        case 2:
            display.setCursor(4, 4);
            display.print("  WILDFIRE ALERT ");
            break;
        case 3:
            display.setCursor(4, 4);
            display.print(" ! DANGER ! ");
            break;
        }

        // Draw small warning triangle on the right side
        display.drawBitmap(110, 0, warning_icon, 16, 16, SSD1306_BLACK);
    }
    else
    {
        display.setCursor(0, 4); // Center vertically in yellow band
        display.println(" Sensor Readings ");
    }

    // Switch to white text on black for the blue area
    display.setTextColor(SSD1306_WHITE);

    // Start a bit closer to the yellow band to save space
    int yPos = YELLOW_BAND_HEIGHT + 1;

    // Display temperature with threshold indicator
    display.setCursor(0, yPos);
    display.print("Temp: ");
    display.print(temperature);
    display.print(" C");
    if (isTemperatureExceedingThreshold(temperature))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display humidity with threshold indicator
    display.setCursor(0, yPos);
    display.print("Humidity: ");
    display.print(humidity);
    display.print(" %");
    if (isHumidityBelowThreshold(humidity))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display smoke value with threshold indicator
    display.setCursor(0, yPos);
    display.print("Smoke: ");
    display.print(smokeValue);
    if (isSmokeExceedingThreshold(smokeValue))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display IR temperature with threshold indicator
    display.setCursor(0, yPos);
    display.print("IR Temp: ");
    display.print(irTemperature);
    display.print(" C");
    if (isIRTemperatureExceedingThreshold(irTemperature))
    {
        display.print(" !");
    }
    yPos += 9; // Reduce spacing from 10 to 9

    // Display thresholds exceeded
    display.setCursor(0, yPos);
    int thresholdsExceeded = getNumberOfThresholdsExceeded(temperature, humidity, smokeValue, irTemperature);
    display.print("Alerts: ");
    display.print(thresholdsExceeded);
    display.print("/4");

    // Update the display
    display.display();
}

// Updated to handle individual sensor errors
void displaySensorData(float temperature, float humidity, int smokeValue, float irTemperature,
                       bool wildfireDetected,
                       bool tempValid,
                       bool humidityValid,
                       bool smokeValid,
                       bool irValid)
{
    display.clearDisplay();

    // Define the display bands
    const int YELLOW_BAND_HEIGHT = 16; // 25% of 64 pixels (2 rows of text)

    // Draw the yellow band on top (for title area)
    display.fillRect(0, 0, SCREEN_WIDTH, YELLOW_BAND_HEIGHT, SSD1306_WHITE);

    // Set text properties for title in yellow area (black text on yellow background)
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);

    if (wildfireDetected)
    {
        // Animate the warning text in the yellow band
        int animPhase = (millis() / 200) % 4; // 4 animation phases, changing every 200ms

        switch (animPhase)
        {
        case 0:
            display.setCursor(4, 4);
            display.print("! WILDFIRE ALERT !");
            break;
        case 1:
            display.setCursor(4, 4);
            display.print(" WILDFIRE ALERT  ");
            break;
        case 2:
            display.setCursor(4, 4);
            display.print("  WILDFIRE ALERT ");
            break;
        case 3:
            display.setCursor(4, 4);
            display.print(" ! DANGER ! ");
            break;
        }

        // Draw small warning triangle on the right side
        display.drawBitmap(110, 0, warning_icon, 16, 16, SSD1306_BLACK);
    }
    else
    {
        // Count errors to show in title
        int errorCount = 0;
        if (!tempValid)
            errorCount++;
        if (!humidityValid)
            errorCount++;
        if (!smokeValid)
            errorCount++;
        if (!irValid)
            errorCount++;

        if (errorCount > 0)
        {
            display.setCursor(0, 4);
            display.print(" SENSOR ERRORS: ");
            display.print(errorCount);
        }
        else
        {
            display.setCursor(0, 4);
            display.println(" Sensor Readings ");
        }
    }

    // Switch to white text on black for the rest of display
    display.setTextColor(SSD1306_WHITE);

    // Start a bit closer to the yellow band to save space
    int yPos = YELLOW_BAND_HEIGHT + 1;

    // Display temperature with threshold indicator or error
    display.setCursor(0, yPos);
    display.print("Temp: ");
    if (tempValid)
    {
        display.print(temperature);
        display.print(" C");
        if (isTemperatureExceedingThreshold(temperature))
        {
            display.print(" !");
        }
    }
    else
    {
        display.print("ERROR");
    }
    yPos += 9;

    // Display humidity with threshold indicator or error
    display.setCursor(0, yPos);
    display.print("Humidity: ");
    if (humidityValid)
    {
        display.print(humidity);
        display.print(" %");
        if (isHumidityBelowThreshold(humidity))
        {
            display.print(" !");
        }
    }
    else
    {
        display.print("ERROR");
    }
    yPos += 9;

    // Display smoke value with threshold indicator or error
    display.setCursor(0, yPos);
    display.print("Smoke: ");
    if (smokeValid)
    {
        display.print(smokeValue);
        if (isSmokeExceedingThreshold(smokeValue))
        {
            display.print(" !");
        }
    }
    else
    {
        display.print("ERROR");
    }
    yPos += 9;

    // Display IR temperature with threshold indicator or error
    display.setCursor(0, yPos);
    display.print("IR Temp: ");
    if (irValid)
    {
        display.print(irTemperature);
        display.print(" C");
        if (isIRTemperatureExceedingThreshold(irTemperature))
        {
            display.print(" !");
        }
    }
    else
    {
        display.print("ERROR");
    }
    yPos += 9;

    // Only show thresholds exceeded if sensors are valid
    int validSensorCount = 0;
    int thresholdsExceeded = 0;

    if (tempValid)
    {
        validSensorCount++;
        if (isTemperatureExceedingThreshold(temperature))
            thresholdsExceeded++;
    }
    if (humidityValid)
    {
        validSensorCount++;
        if (isHumidityBelowThreshold(humidity))
            thresholdsExceeded++;
    }
    if (smokeValid)
    {
        validSensorCount++;
        if (isSmokeExceedingThreshold(smokeValue))
            thresholdsExceeded++;
    }
    if (irValid)
    {
        validSensorCount++;
        if (isIRTemperatureExceedingThreshold(irTemperature))
            thresholdsExceeded++;
    }

    if (validSensorCount > 0)
    {
        display.setCursor(0, yPos);
        display.print("Alerts: ");
        display.print(thresholdsExceeded);
        display.print("/");
        display.print(validSensorCount);
    }

    // Update the display
    display.display();
}

void displayInitStatus(const String &message, int progress)
{
    display.clearDisplay();

    // Draw title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("FireShield 360");
    display.println("Initializing...");

    // Draw horizontal line
    display.drawLine(0, 16, SCREEN_WIDTH, 16, SSD1306_WHITE);

    // Display the status message
    display.setCursor(0, 20);
    display.println(message);

    // Draw progress bar if progress value is provided
    if (progress >= 0 && progress <= 100)
    {
        // Draw progress bar outline
        display.drawRect(0, 44, SCREEN_WIDTH, 10, SSD1306_WHITE);

        // Draw progress bar fill
        int fillWidth = (progress * (SCREEN_WIDTH - 4)) / 100;
        display.fillRect(2, 46, fillWidth, 6, SSD1306_WHITE);

        // Display percentage
        display.setCursor(50, 56);
        display.print(progress);
        display.print("%");
    }

    display.display();
}

void displaySensorCheck(const String &sensorName)
{
    display.clearDisplay();

    // Draw title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("FireShield 360");

    // Draw horizontal line
    display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

    // Display checking message
    display.setCursor(0, 16);
    display.print("Checking ");
    display.println(sensorName);

    // Draw blinking dots to indicate activity
    int dots = (millis() / 500) % 4; // 0-3 dots blinking
    display.setCursor(0, 28);
    for (int i = 0; i < dots; i++)
    {
        display.print(". ");
    }

    // Draw a small animation in the center
    int frame = (millis() / 300) % 3;
    switch (frame)
    {
    case 0:
        display.fillCircle(64, 40, 6, SSD1306_WHITE);
        break;
    case 1:
        display.drawCircle(64, 40, 8, SSD1306_WHITE);
        break;
    case 2:
        display.drawCircle(64, 40, 10, SSD1306_WHITE);
        break;
    }

    display.display();
}

void displayPowerOff()
{
    // First phase - Show power icon with full screen
    for (int i = 0; i <= 5; i++)
    {
        display.clearDisplay();

        // Draw title with reducing brightness
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(14, 10);
        display.println("FireShield 360");

        // Draw horizontal line
        display.drawLine(0, 20, SCREEN_WIDTH, 20, SSD1306_WHITE);

        // Show power icon in center of screen
        display.drawBitmap(48, 24, power_icon, 32, 32, SSD1306_WHITE);

        display.display();
        delay(50);
    }

    // Second phase - Show powering off animation
    for (int i = 0; i < 5; i++)
    {
        display.clearDisplay();

        // Draw title that fades away
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(14, 10);
        display.println("FireShield 360");

        // Draw horizontal line
        display.drawLine(0, 20, SCREEN_WIDTH, 20, SSD1306_WHITE);

        // Draw power icon (alternating to create blink effect)
        if (i % 2 == 0)
        {
            display.drawBitmap(48, 24, power_icon, 32, 32, SSD1306_WHITE);
        }

        // Draw "Powering Off" text
        display.setCursor(24, 56);
        display.println("POWERING OFF...");

        display.display();
        delay(250);
    }

    // Final phase - Fade out animation
    for (int fadeStep = 0; fadeStep < 4; fadeStep++)
    {
        display.clearDisplay();

        // Only draw elements based on fade step to create fading effect
        if (fadeStep < 3)
        {
            display.drawBitmap(48, 24, power_icon, 32, 32, SSD1306_WHITE);
        }

        if (fadeStep < 2)
        {
            display.setTextSize(1);
            display.setCursor(24, 56);
            display.println("POWERING OFF...");
        }

        display.display();
        delay(300);
    }

    // Final black screen for powered off state
    display.clearDisplay();
    display.display();
    delay(200);

    // Show final hint - very dim
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(8, 28);
    display.println("Switch ON to restart");
    display.display();
}