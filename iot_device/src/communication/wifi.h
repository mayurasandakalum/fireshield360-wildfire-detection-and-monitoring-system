#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>

// Default built-in LED pin for ESP32
#define WIFI_STATUS_LED 2

// Initialize WiFi connection with LED indication
void initWiFi(const char *ssid, const char *password, int statusLedPin = WIFI_STATUS_LED);

// Check WiFi connection and reconnect if needed
void ensureWiFiConnected();

// Get connection status
bool isWiFiConnected();

// Update LED based on connection status
void updateWifiStatusLed();

#endif // WIFI_H
