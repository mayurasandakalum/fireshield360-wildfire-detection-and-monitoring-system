#include <Arduino.h>
#include <ArduinoJson.h>
#include "mqtt.h"
#include "wifi.h"
#include "time_sync.h"

// MQTT client objects
WiFiClientSecure espClient;
PubSubClient client(espClient);

// MQTT credentials
const char *mqtt_server_global;
int mqtt_port_global;
const char *mqtt_username_global;
const char *mqtt_password_global;

void initMQTT(const char *server, int port, const char *username, const char *password)
{
    mqtt_server_global = server;
    mqtt_port_global = port;
    mqtt_username_global = username;
    mqtt_password_global = password;

    // Configure the secure client
    espClient.setInsecure(); // For testing; use setCACert() with the CA certificate in production

    // Set MQTT server
    client.setServer(server, port);

    Serial.println("MQTT client initialized");
}

bool ensureMQTTConnected()
{
    // First ensure WiFi is connected
    ensureWiFiConnected();

    if (!client.connected() && isWiFiConnected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "FireShield360-";
        clientId += String(random(0xffff), HEX);

        // Attempt to connect with MQTT username and password
        if (client.connect(clientId.c_str(), mqtt_username_global, mqtt_password_global))
        {
            Serial.println("connected");
            return true;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" will try again next time");
            return false;
        }
    }

    return client.connected();
}

bool publishSensorData(float temperature, float humidity, int smoke, bool smokeDetected, const char *topic)
{
    if (!ensureMQTTConnected())
    {
        return false;
    }

    // Create a JSON document using non-deprecated constructor
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["smoke"] = smoke;
    doc["smoke_detected"] = smokeDetected;

    // Add properly formatted timestamp
    char timeBuffer[25];
    getFormattedTime(timeBuffer, sizeof(timeBuffer));
    doc["timestamp"] = timeBuffer;

    doc["device_id"] = "esp32_01";

    // Serialize JSON to a string
    char buffer[256];
    serializeJson(doc, buffer);

    // Publish the message
    bool result = client.publish(topic, buffer);

    if (result)
    {
        Serial.println("Data published to MQTT:");
        Serial.println(buffer);
    }
    else
    {
        Serial.println("Failed to publish data to MQTT");
    }

    return result;
}

// New function for publishing sensor data without smoke detection flag
bool publishSensorData(float temperature, float humidity, int smoke, const char *topic)
{
    if (!ensureMQTTConnected())
    {
        return false;
    }

    // Create a JSON document
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["smoke"] = smoke;

    // Add properly formatted timestamp
    char timeBuffer[25];
    getFormattedTime(timeBuffer, sizeof(timeBuffer));
    doc["timestamp"] = timeBuffer;

    doc["device_id"] = "esp32_01";

    // Serialize JSON to a string
    char buffer[256];
    serializeJson(doc, buffer);

    // Publish the message
    bool result = client.publish(topic, buffer);

    if (result)
    {
        Serial.print("Data published to MQTT: ");
        Serial.println(topic);
        Serial.println(buffer);
    }
    else
    {
        Serial.println("Failed to publish data to MQTT");
    }

    return result;
}

bool isMQTTConnected()
{
    return client.connected();
}

void processMQTTMessages()
{
    if (client.connected())
    {
        client.loop();
    }
}
