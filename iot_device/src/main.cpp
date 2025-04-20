#include <Arduino.h>
#include "sensors/temperature.h"
#include "sensors/humidity.h"
#include "sensors/smoke.h"
#include "communication/wifi.h"
#include "communication/mqtt.h"
#include "communication/time_sync.h"
#include "actuators/led.h"
#include "display/oled_display.h"
#include "detection/wildfire_detection.h" // Add wildfire detection module

// Sensor pins
const int DHT_PIN = 4;       // GPIO4 for DHT11 sensor
const int SMOKE_PIN = 34;    // GPIO34 for MQ2 sensor
const int WIFI_LED_PIN = 27; // GPIO27 for WiFi status LED

// Sensor status LED pins
const int TEMP_HUM_LED_PIN = 12; // GPIO12 for temperature/humidity sensor status
const int SMOKE_LED_PIN = 13;    // GPIO13 for smoke sensor status

// Add LED pin for wildfire alert
const int WILDFIRE_ALERT_LED_PIN = 14; // GPIO14 for wildfire alert LED

// Variables to store sensor values
float temperature = 0.0;
float humidity = 0.0;
int smokeValue = 0;

// Add a variable to track wildfire detection
bool wildfireDetected = false;

// WiFi credentials
const char *ssid = "NeuralNet";
const char *password = "camtasia";

// MQTT Broker details
const char *mqtt_server = "7ce36aef28e949f4b384e4808389cffc.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_topic = "esp32_01/sensors/data";

// MQTT credentials
const char *mqtt_username = "hivemq.webclient.1745152369573";
const char *mqtt_password = "f%h3.X!9WGm0HtoJ1sO&";

// Last time data was sent
unsigned long lastPublishTime = 0;
const long publishInterval = 1000; // Publish data every 1 seconds

// Last time display was updated
unsigned long lastDisplayTime = 0;
const long displayInterval = 1000; // Update display every 1 seconds

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("FireShield 360 - Temperature, Humidity and Smoke Monitoring");

  // Initialize sensor status LEDs using the LED module
  initLed(TEMP_HUM_LED_PIN);
  initLed(SMOKE_LED_PIN);
  initLed(WILDFIRE_ALERT_LED_PIN); // Add wildfire alert LED
  Serial.println("Sensor status LEDs initialized on pins 12, 13 and 14");

  // Initialize OLED display
  if (initDisplay())
  {
    Serial.println("OLED display initialized successfully");
    displaySplashScreen(3000); // Show splash screen for 3 seconds
  }
  else
  {
    Serial.println("Failed to initialize OLED display");
  }

  // Initialize temperature & humidity sensors (DHT11)
  Serial.println("Initializing DHT11 sensor...");
  initTemperatureSensor(DHT_PIN);
  initHumiditySensor(DHT_PIN);

  // Initialize smoke sensor (MQ2)
  Serial.println("Initializing MQ2 smoke sensor...");
  initSmokeSensor(SMOKE_PIN);

  // Initialize wildfire detection with default thresholds
  initWildfireDetection();

  // Initialize WiFi with external LED on GPIO 5
  initWiFi(ssid, password, WIFI_LED_PIN);

  // Initialize time synchronization
  initTimeSync();

  // Initialize MQTT
  initMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_password);

  // Wait for the sensor to stabilize
  delay(2000);

  // Show a welcome message on the display
  textDisplay("FireShield 360");

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

  // Check for wildfire conditions
  wildfireDetected = isWildfireDetected(temperature, humidity, smokeValue);

  // Update wildfire alert LED
  if (wildfireDetected)
  {
    // Blink the LED for wildfire alert
    if ((millis() / 500) % 2 == 0)
    {
      turnOnLed(WILDFIRE_ALERT_LED_PIN);
    }
    else
    {
      turnOffLed(WILDFIRE_ALERT_LED_PIN);
    }
  }
  else
  {
    turnOffLed(WILDFIRE_ALERT_LED_PIN);
  }

  // Update OLED display at regular intervals
  unsigned long currentTime = millis();
  if (currentTime - lastDisplayTime >= displayInterval)
  {
    lastDisplayTime = currentTime;
    if (isTemperatureReadingValid() && isHumidityReadingValid() && isSmokeReadingValid())
    {
      displaySensorData(temperature, humidity, smokeValue, wildfireDetected);
    }
    else
    {
      textDisplay("Sensor Error!");
    }
  }

  // Check if readings are valid
  if (isTemperatureReadingValid() && isHumidityReadingValid())
  {
    // Publish data to MQTT at regular intervals
    if (currentTime - lastPublishTime >= publishInterval)
    {
      lastPublishTime = currentTime;

      // Publish to MQTT with wildfire detection status
      publishSensorData(temperature, humidity, smokeValue, wildfireDetected, mqtt_topic);
    }
  }
  else
  {
    Serial.println("Failed to get valid readings from DHT11");
  }

  // Wait before next reading
  delay(1000);
}
