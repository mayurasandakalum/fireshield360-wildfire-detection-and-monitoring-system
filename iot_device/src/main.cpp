#include <Arduino.h>
#include "sensors/temperature.h"
#include "sensors/humidity.h"
#include "communication/wifi.h"
#include "communication/mqtt.h"
#include "communication/time_sync.h"

// DHT11 sensor pin
const int DHT_PIN = 4; // GPIO4 for DHT11 sensor

// Variables to store sensor values
float temperature = 0.0;
float humidity = 0.0;

// WiFi credentials
const char *ssid = "NeuralNet";
const char *password = "camtasia";

// MQTT Broker details
const char *mqtt_server = "2b62039e7b4e4cec8887f544d74f7281.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_topic = "/sensors/esp32_01/dht11/data";

// MQTT credentials
const char *mqtt_username = "hivemq.webclient.1742497701403";
const char *mqtt_password = ";UGpfs>13Thun4P@2HI:";

// Last time data was sent
unsigned long lastPublishTime = 0;
const long publishInterval = 5000; // Publish data every 5 seconds

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("FireShield 360 - Temperature and Humidity Monitoring");

  // Initialize temperature & humidity sensors (DHT11)
  Serial.println("Initializing DHT11 sensor...");
  initTemperatureSensor(DHT_PIN);
  initHumiditySensor(DHT_PIN);

  // Initialize WiFi with built-in LED indication
  initWiFi(ssid, password);

  // Initialize time synchronization
  initTimeSync();

  // Initialize MQTT
  initMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_password);

  // Wait for the sensor to stabilize
  delay(2000);

  Serial.println("System initialized and ready.");
}

void loop()
{
  // Check WiFi status and update LED
  ensureWiFiConnected();

  // If WiFi just reconnected, sync time again
  static bool wasConnected = false;
  if (isWiFiConnected() && !wasConnected)
  {
    syncTime();
  }
  wasConnected = isWiFiConnected();

  // Read sensors
  temperature = readTemperature();
  humidity = readHumidity();

  // Process MQTT messages
  processMQTTMessages();

  // Check if readings are valid
  if (isTemperatureReadingValid() && isHumidityReadingValid())
  {
    // Publish data to MQTT at regular intervals
    unsigned long currentTime = millis();
    if (currentTime - lastPublishTime >= publishInterval)
    {
      lastPublishTime = currentTime;

      // Publish to MQTT
      publishSensorData(temperature, humidity, mqtt_topic);
    }
  }
  else
  {
    Serial.println("Failed to get valid readings from DHT11");
  }

  // Wait before next reading
  delay(3000);
}
