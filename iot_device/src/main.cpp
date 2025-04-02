#include <Arduino.h>
#include "sensors/temperature.h"
#include "sensors/humidity.h"
#include "sensors/smoke.h"
#include "communication/wifi.h"
#include "communication/mqtt.h"
#include "communication/time_sync.h"
#include "actuators/led.h" // Include the LED module

// Sensor pins
const int DHT_PIN = 4;      // GPIO4 for DHT11 sensor
const int SMOKE_PIN = 34;   // GPIO34 for MQ2 sensor
const int WIFI_LED_PIN = 5; // GPIO5 for WiFi status LED

// Sensor status LED pins
const int TEMP_HUM_LED_PIN = 12; // GPIO12 for temperature/humidity sensor status
const int SMOKE_LED_PIN = 13;    // GPIO13 for smoke sensor status

// Variables to store sensor values
float temperature = 0.0;
float humidity = 0.0;
int smokeValue = 0;

// WiFi credentials
const char *ssid = "NeuralNet";
const char *password = "camtasia";

// MQTT Broker details
const char *mqtt_server = "0a3995fec7984525ba12af5d3b4b7323.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_topic = "/sensors/esp32_01/dht11/data";

// MQTT credentials
const char *mqtt_username = "hivemq.webclient.1742623130057";
const char *mqtt_password = "3K<,d5afbCPRAr.0$4Dl";

// Last time data was sent
unsigned long lastPublishTime = 0;
const long publishInterval = 5000; // Publish data every 5 seconds

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("FireShield 360 - Temperature, Humidity and Smoke Monitoring");

  // Initialize sensor status LEDs using the LED module
  initLed(TEMP_HUM_LED_PIN);
  initLed(SMOKE_LED_PIN);
  Serial.println("Sensor status LEDs initialized on pins 12 and 13");

  // Initialize temperature & humidity sensors (DHT11)
  Serial.println("Initializing DHT11 sensor...");
  initTemperatureSensor(DHT_PIN);
  initHumiditySensor(DHT_PIN);

  // Initialize smoke sensor (MQ2)
  Serial.println("Initializing MQ2 smoke sensor...");
  initSmokeSensor(SMOKE_PIN);

  // Initialize WiFi with external LED on GPIO 5
  initWiFi(ssid, password, WIFI_LED_PIN);

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
  smokeValue = readSmokeValue();

  // Update sensor status LEDs using the LED module
  if (isTemperatureReadingValid() && isHumidityReadingValid())
  {
    turnOnLed(TEMP_HUM_LED_PIN);
  }
  else
  {
    turnOffLed(TEMP_HUM_LED_PIN);
  }

  if (isSmokeReadingValid())
  {
    turnOnLed(SMOKE_LED_PIN);
  }
  else
  {
    turnOffLed(SMOKE_LED_PIN);
  }

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

      // Check if smoke is detected
      bool smokeDetected = isSmokeDetected();
      if (smokeDetected)
      {
        Serial.println("ALERT: Smoke detected!");
      }

      // Publish to MQTT with smoke data
      publishSensorData(temperature, humidity, smokeValue, smokeDetected, mqtt_topic);
    }
  }
  else
  {
    Serial.println("Failed to get valid readings from DHT11");
  }

  // Wait before next reading
  delay(3000);
}
