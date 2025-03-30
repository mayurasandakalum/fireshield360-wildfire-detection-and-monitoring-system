#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Initialize MQTT client
void initMQTT(const char *server, int port, const char *username, const char *password);

// Ensure MQTT connection is active
bool ensureMQTTConnected();

// Publish temperature and humidity data
bool publishSensorData(float temperature, float humidity, const char *topic);

// Get connection status
bool isMQTTConnected();

// Process MQTT messages (must be called regularly)
void processMQTTMessages();

#endif // MQTT_H
