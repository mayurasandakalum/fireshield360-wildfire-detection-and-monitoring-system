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

// Publish temperature, humidity and smoke data (without smoke detection)
bool publishSensorData(float temperature, float humidity, int smoke, const char *topic);

// Publish temperature, humidity and smoke data
bool publishSensorData(float temperature, float humidity, int smoke, bool smokeDetected, const char *topic);

// Publish temperature, humidity and smoke data with wildfire detection status
bool publishSensorData(float temperature, float humidity, int smoke, bool wildfireDetected, const char *topic);

// Get connection status
bool isMQTTConnected();

// Process MQTT messages (must be called regularly)
void processMQTTMessages();

#endif // MQTT_H
