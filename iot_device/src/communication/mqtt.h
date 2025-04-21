#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Initialize MQTT client
void initMQTT(const char *server, int port, const char *username, const char *password);

// Ensure MQTT is connected
bool ensureMQTTConnected();

// Process incoming MQTT messages
void processMQTTMessages();

// Publish sensor data to MQTT (updated to include IR temperature)
bool publishSensorData(float temperature, float humidity, int smokeValue, float irTemperature, bool wildfireDetected, const char *topic);

// Publish wildfire alert to MQTT (updated to include IR temperature)
bool publishWildfireAlert(float temperature, float humidity, int smokeValue, float irTemperature,
                          int thresholdsExceeded, const char *topic);

// Subscribe to a topic
bool subscribeTopic(const char *topic);

// Unsubscribe from a topic
bool unsubscribeTopic(const char *topic);

#endif // MQTT_H
