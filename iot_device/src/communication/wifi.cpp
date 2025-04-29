#include <Arduino.h>
#include "wifi.h"

// WiFi credentials
const char *ssid_global;
const char *password_global;
int wifi_status_led_pin = WIFI_STATUS_LED;

void initWiFi(const char *ssid, const char *password, int statusLedPin)
{
    ssid_global = ssid;
    password_global = password;
    wifi_status_led_pin = statusLedPin;

    // Initialize the LED pin
    pinMode(wifi_status_led_pin, OUTPUT);
    digitalWrite(wifi_status_led_pin, LOW); // Initially off

    Serial.print("WiFi status LED configured on pin ");
    Serial.println(wifi_status_led_pin);

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(wifi_status_led_pin, HIGH); // Turn LED on when connected
    }
    else
    {
        Serial.println("\nFailed to connect to WiFi");
        digitalWrite(wifi_status_led_pin, LOW); // Ensure LED is off
    }
}

void ensureWiFiConnected()
{
    bool wasConnected = isWiFiConnected();

    if (!wasConnected)
    {
        Serial.println("WiFi connection lost. Reconnecting...");
        WiFi.begin(ssid_global, password_global);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10)
        {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nWiFi reconnected");
        }
        else
        {
            Serial.println("\nFailed to reconnect to WiFi");
        }
    }

    // Update LED status based on current connection state
    updateWifiStatusLed();
}

bool isWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void updateWifiStatusLed()
{
    if (isWiFiConnected())
    {
        digitalWrite(wifi_status_led_pin, HIGH); // LED on when connected
    }
    else
    {
        digitalWrite(wifi_status_led_pin, LOW); // LED off when disconnected
    }
}

void disconnectWiFi()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi disconnected and turned off");
}
