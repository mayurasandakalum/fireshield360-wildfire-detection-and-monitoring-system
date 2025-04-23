#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h> // Add Wire.h include for I2C communication
#include "sensors/temperature.h"
#include "sensors/humidity.h"
#include "sensors/smoke.h"
#include "sensors/infrared.h" // Add infrared sensor include
#include "communication/wifi.h"
#include "communication/mqtt.h"
#include "communication/time_sync.h"
#include "actuators/led.h"
#include "display/oled_display.h"
#include "detection/wildfire_detection.h"

// Sensor pins
const int DHT_PIN = 4;       // GPIO4 for DHT11 sensor
const int SMOKE_PIN = 34;    // GPIO34 for MQ2 sensor
const int WIFI_LED_PIN = 27; // GPIO27 for WiFi status LED

// RGB LED pins for temperature/humidity sensor
const int TEMP_HUM_RED_PIN = 13;   // GPIO13 for RGB red
const int TEMP_HUM_GREEN_PIN = 12; // GPIO12 for RGB green
const int TEMP_HUM_BLUE_PIN = 14;  // GPIO14 for RGB blue

// Temperature and humidity thresholds for RGB LED status
const float TEMP_HIGH_THRESHOLD = 35.0; // Temperature > 35°C: High warning (Red)
const float TEMP_LOW_THRESHOLD = 15.0;  // Temperature < 15°C: Low warning (Blue)
const float HUM_HIGH_THRESHOLD = 70.0;  // Humidity > 70%: High warning (Red)
const float HUM_LOW_THRESHOLD = 30.0;   // Humidity < 30%: Low warning (Blue)

// Sensor status LED pins
const int SMOKE_LED_PIN = 25; // GPIO25 for smoke sensor status

// Add LED pin for wildfire alert
const int WILDFIRE_ALERT_LED_PIN = 26; // GPIO26 for wildfire alert LED

// Variables to store sensor values
float temperature = 0.0;
float humidity = 0.0;
int smokeValue = 0;
float irTemperature = 0.0; // Add variable for infrared temperature

// Add a variable to track wildfire detection
bool wildfireDetected = false;

// WiFi credentials
const char *ssid = "NeuralNet";
const char *password = "camtasia";

// MQTT Broker details
const char *mqtt_server = "7ce36aef28e949f4b384e4808389cffc.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_topic = "esp32_01/sensors/data";
const char *mqtt_wildfire_topic = "esp32_01/wildfire/alert"; // Add new topic for wildfire alerts

// MQTT credentials
const char *mqtt_username = "hivemq.webclient.1745152369573";
const char *mqtt_password = "f%h3.X!9WGm0HtoJ1sO&";

// Last time data was sent
unsigned long lastPublishTime = 0;
const long publishInterval = 1000; // Publish data every 1 seconds

// Last time display was updated
unsigned long lastDisplayTime = 0;
const long displayInterval = 1000; // Update display every 1 seconds

// Last time wildfire alert was sent
unsigned long lastWildfireAlertTime = 0;
const long wildfireAlertInterval = 60000; // Send alert every 60 seconds when wildfire detected

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("FireShield 360 - Temperature, Humidity and Smoke Monitoring");

  // Initialize I2C communication
  Wire.begin(21, 22); // SDA on GPIO 21, SCL on GPIO 22
  Serial.println("I2C initialized with SDA=21, SCL=22");

  // Initialize RGB LED for temperature/humidity sensor
  initRgbLed(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN);

  // Show initialization sequence - blue blinking
  showSensorInitializationSequence(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN);

  // Initialize sensor status LEDs using the LED module
  initLed(SMOKE_LED_PIN);
  initLed(WILDFIRE_ALERT_LED_PIN);
  Serial.println("Sensor status LEDs initialized");

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

  // Initialize infrared temperature sensor (MLX90614)
  Serial.println("Initializing MLX90614 infrared sensor...");
  initInfraredSensor();

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
  irTemperature = readObjectTemperature();

  // Update temperature/humidity sensor status with RGB LED
  bool tempHumValid = isTemperatureReadingValid() && isHumidityReadingValid();
  bool sensorStatus = updateSensorStatusLed(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN, tempHumValid);

  // Debug info for sensor errors
  if (!sensorStatus)
  {
    if (!isTemperatureReadingValid() && !isHumidityReadingValid())
    {
      Serial.println("DHT11 ERROR: Both temperature and humidity sensors disconnected or failed");
    }
    else if (!isTemperatureReadingValid())
    {
      Serial.println("DHT11 ERROR: Temperature sensor disconnected or failed");
    }
    else
    {
      Serial.println("DHT11 ERROR: Humidity sensor disconnected or failed");
    }
  }

  // Update smoke sensor status LED
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

  // Check for wildfire conditions - now includes IR temperature
  wildfireDetected = isWildfireDetected(temperature, humidity, smokeValue, irTemperature);

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

    // Send wildfire alert via MQTT at regular intervals
    unsigned long currentTime = millis();
    if (currentTime - lastWildfireAlertTime >= wildfireAlertInterval)
    {
      lastWildfireAlertTime = currentTime;

      // Use the dedicated function from mqtt.h to publish the wildfire alert
      if (publishWildfireAlert(temperature, humidity, smokeValue, irTemperature,
                               getNumberOfThresholdsExceeded(temperature, humidity, smokeValue, irTemperature),
                               mqtt_wildfire_topic))
      {
        Serial.println("Wildfire alert published to MQTT");
      }
      else
      {
        Serial.println("Failed to publish wildfire alert");
      }
    }
  }
  else
  {
    turnOffLed(WILDFIRE_ALERT_LED_PIN);
    lastWildfireAlertTime = 0; // Reset the timer when no wildfire is detected
  }

  // Update OLED display at regular intervals
  unsigned long currentTime = millis();
  if (currentTime - lastDisplayTime >= displayInterval)
  {
    lastDisplayTime = currentTime;
    if (isTemperatureReadingValid() && isHumidityReadingValid() && isSmokeReadingValid() && isInfraredReadingValid())
    {
      displaySensorData(temperature, humidity, smokeValue, irTemperature, wildfireDetected);
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
      publishSensorData(temperature, humidity, smokeValue, irTemperature, wildfireDetected, mqtt_topic);
    }
  }
  else
  {
    Serial.println("Failed to get valid readings from DHT11");
  }

  // Wait before next reading
  delay(1000);
}
