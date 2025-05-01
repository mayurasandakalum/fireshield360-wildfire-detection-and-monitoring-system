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

void setMQTTCallback(void (*callback)(char *, byte *, unsigned int))
{
    client.setCallback(callback);
    Serial.println("MQTT callback function set");
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

// Original version without IR temperature - kept for backward compatibility
bool publishSensorData(float temperature, float humidity, int smoke, bool wildfireDetected, const char *topic)
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
    // Removed smoke_detected field as requested
    doc["potential_wildfire"] = wildfireDetected; // Changed from wildfire_detected to potential_wildfire

    // Add properly formatted timestamp
    char timeBuffer[25];
    getFormattedTime(timeBuffer, sizeof(timeBuffer));
    doc["timestamp"] = timeBuffer;

    doc["device_id"] = "esp32_01";

    // Add number of thresholds exceeded
    int thresholdsExceeded = 0;
    if (temperature > 40.0)
        thresholdsExceeded++;
    if (humidity < 30.0)
        thresholdsExceeded++;
    if (smoke > 1500)
        thresholdsExceeded++;
    doc["thresholds_exceeded"] = thresholdsExceeded;

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

// New version with IR temperature
bool publishSensorData(float temperature, float humidity, int smokeValue, float irTemperature, bool wildfireDetected, const char *topic)
{
    if (!ensureMQTTConnected())
    {
        return false;
    }

    // Create a JSON document using non-deprecated constructor
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["smoke"] = smokeValue;
    doc["ir_temperature"] = irTemperature;        // Add IR temperature
    doc["potential_wildfire"] = wildfireDetected; // Changed from wildfire_detected to potential_wildfire

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

// Original version without smoke detection flag - kept for backward compatibility
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
        Serial.println("");
    }
    else
    {
        Serial.println("Failed to publish data to MQTT");
    }

    return result;
}

// Original version without IR temperature - kept for backward compatibility
bool publishWildfireAlert(float temperature, float humidity, int smoke, int thresholdsExceeded, const char *topic)
{
    if (!ensureMQTTConnected())
    {
        return false;
    }

    // Create a JSON document for the alert
    JsonDocument alertDoc;
    alertDoc["device_id"] = "esp32_01";
    alertDoc["potential_wildfire"] = true; // Changed from wildfire_detected to potential_wildfire
    alertDoc["temperature"] = temperature;
    alertDoc["humidity"] = humidity;
    alertDoc["smoke"] = smoke;
    alertDoc["thresholds_exceeded"] = thresholdsExceeded;

    // Add properly formatted timestamp
    char timeBuffer[25];
    getFormattedTime(timeBuffer, sizeof(timeBuffer));
    alertDoc["timestamp"] = timeBuffer;

    // Serialize JSON to a string
    char buffer[256];
    serializeJson(alertDoc, buffer);

    // Publish the message
    bool result = client.publish(topic, buffer);

    if (result)
    {
        Serial.println("Wildfire alert published to MQTT:");
        Serial.println(buffer);
    }
    else
    {
        Serial.println("Failed to publish wildfire alert to MQTT");
    }

    return result;
}

// New version with IR temperature
bool publishWildfireAlert(float temperature, float humidity, int smokeValue, float irTemperature,
                          int thresholdsExceeded, const char *topic)
{
    if (!ensureMQTTConnected())
    {
        return false;
    }

    // Create a JSON document for the alert
    JsonDocument alertDoc;
    alertDoc["device_id"] = "esp32_01";
    alertDoc["potential_wildfire"] = true; // Changed from wildfire_detected to potential_wildfire
    alertDoc["temperature"] = temperature;
    alertDoc["humidity"] = humidity;
    alertDoc["smoke"] = smokeValue;
    alertDoc["ir_temperature"] = irTemperature; // Add IR temperature
    alertDoc["thresholds_exceeded"] = thresholdsExceeded;

    // Add properly formatted timestamp
    char timeBuffer[25];
    getFormattedTime(timeBuffer, sizeof(timeBuffer));
    alertDoc["timestamp"] = timeBuffer;

    // Serialize JSON to a string
    char buffer[256];
    serializeJson(alertDoc, buffer);

    // Publish the message
    bool result = client.publish(topic, buffer);

    if (result)
    {
        Serial.println("Wildfire alert published to MQTT:");
        Serial.println(buffer);
    }
    else
    {
        Serial.println("Failed to publish wildfire alert to MQTT");
    }

    return result;
}

bool subscribeTopic(const char *topic)
{
    if (!ensureMQTTConnected())
    {
        return false;
    }

    bool result = client.subscribe(topic);

    if (result)
    {
        Serial.print("Subscribed to topic: ");
        Serial.println(topic);
    }
    else
    {
        Serial.print("Failed to subscribe to topic: ");
        Serial.println(topic);
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

void disconnectMQTT()
{
    if (client.connected())
    {
        client.disconnect();
        Serial.println("Disconnected from MQTT broker");
    }
}

void reconnectMQTT()
{
    // Try to reconnect to the broker
    Serial.println("Reconnecting to MQTT broker...");
    if (client.connect("ESP32Client", mqtt_username_global, mqtt_password_global))
    {
        Serial.println("Reconnected to MQTT broker");
    }
    else
    {
        Serial.print("Failed to reconnect to MQTT broker, rc=");
        Serial.println(client.state());
    }
}
