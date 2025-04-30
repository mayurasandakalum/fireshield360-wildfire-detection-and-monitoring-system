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
#include "actuators/buzzer.h" // Add buzzer include
#include "display/oled_display.h"
#include "detection/wildfire_detection.h"

// Forward declarations
void mqttCallback(char *topic, byte *payload, unsigned int length);
void reinitializeMQTTSubscriptions();

// Sensor pins
const int DHT_PIN = 4;           // GPIO4 for DHT11 sensor
const int SMOKE_PIN = 34;        // GPIO34 for MQ2 sensor
const int POWER_SWITCH_PIN = 35; // GPIO35 for power switch (input-only pin)

// RGB LED pins for temperature/humidity sensor
const int TEMP_HUM_RED_PIN = 13;   // GPIO13 for RGB red
const int TEMP_HUM_GREEN_PIN = 12; // GPIO12 for RGB green
const int TEMP_HUM_BLUE_PIN = 14;  // GPIO14 for RGB blue

// RGB LED pins for infrared temperature sensor
const int IR_RED_PIN = 25;   // GPIO25 for RGB red
const int IR_GREEN_PIN = 26; // GPIO26 for RGB green
const int IR_BLUE_PIN = 27;  // GPIO27 for RGB blue

// RGB LED pins for smoke sensor
// NOTE: GPIO35 is input-only and can't be used for output. Changed to GPIO15
const int SMOKE_RED_PIN = 15;   // Changed from 35 to 15 - GPIO15 for RGB red
const int SMOKE_GREEN_PIN = 32; // GPIO32 for RGB green
const int SMOKE_BLUE_PIN = 33;  // GPIO33 for RGB blue

// Note: We need to reassign the following single-color LEDs to avoid pin conflicts
// Previously SMOKE_LED_PIN was 32 (now used for SMOKE_BLUE_PIN)
// Previously WILDFIRE_ALERT_LED_PIN was 33 (now used for SMOKE_GREEN_PIN)

// Assign new pins for single-color LEDs - avoiding conflicts with all RGB LEDs
const int WILDFIRE_ALERT_LED_PIN = 5; // Reassigned to GPIO5
const int WIFI_LED_PIN = 2;           // Using built-in LED on GPIO2

// Define buzzer pin
const int BUZZER_PIN = 19; // GPIO19 for buzzer

// Temperature and humidity thresholds for RGB LED status
const float TEMP_HIGH_THRESHOLD = 35.0; // Temperature > 35°C: High warning (Red)
const float TEMP_LOW_THRESHOLD = 15.0;  // Temperature < 15°C: Low warning (Blue)
const float HUM_HIGH_THRESHOLD = 70.0;  // Humidity > 70%: High warning (Red)
const float HUM_LOW_THRESHOLD = 30.0;   // Humidity < 30%: Low warning (Blue)

// Variables to store sensor values
float temperature = 0.0;
float humidity = 0.0;
int smokeValue = 0;
float irTemperature = 0.0; // Add variable for infrared temperature

// Add a variable to track wildfire detection
bool wildfireDetected = false;

// Add a variable to track verified wildfire detection
bool verifiedWildfireDetected = false;

// WiFi credentials
const char *ssid = "NeuralNet";
const char *password = "camtasia";

// MQTT Broker details
const char *mqtt_server = "7ce36aef28e949f4b384e4808389cffc.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_topic = "esp32_01/sensors/data"; // Removed dedicated wildfire topic

// Define the verification topic
const char *verification_topic = "esp32_01/verified/status";

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

// Standard buzzer sound duration
const unsigned long BUZZER_ALERT_DURATION = 10000; // 10 seconds duration for buzzer alerts

// Simplified isPowerOn function without debouncing
bool isPowerOn()
{
  // Simply read the current state of the switch and return it
  // (LOW means ON since it's connected to GND when switch is on)
  return digitalRead(POWER_SWITCH_PIN) == LOW;
}

void showSensorInitializationSequence(int redPin, int greenPin, int bluePin, const String &sensorName)
{
  // Display sensor check on OLED
  displaySensorCheck(sensorName);

  // Turn on blue LED and wait
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
  delay(300);

  // Update display while blinking LED
  displaySensorCheck(sensorName);

  // Turn off LED and wait
  digitalWrite(bluePin, LOW);
  delay(200);

  // Update display again
  displaySensorCheck(sensorName);

  // Turn on blue LED and wait
  digitalWrite(bluePin, HIGH);
  delay(300);

  // Update display
  displaySensorCheck(sensorName);

  // Keep blue LED on to indicate sensor is being checked
  // (Don't turn off LED at the end of checking)
}

// New function to set all sensor LEDs to green when initialization is complete
void setAllSensorsToGreen()
{
  // Set Temperature/Humidity LED to green
  digitalWrite(TEMP_HUM_RED_PIN, LOW);
  digitalWrite(TEMP_HUM_GREEN_PIN, HIGH);
  digitalWrite(TEMP_HUM_BLUE_PIN, LOW);

  // Set IR Temperature LED to green
  digitalWrite(IR_RED_PIN, LOW);
  digitalWrite(IR_GREEN_PIN, HIGH);
  digitalWrite(IR_BLUE_PIN, LOW);

  // Set Smoke Sensor LED to green
  digitalWrite(SMOKE_RED_PIN, LOW);
  digitalWrite(SMOKE_GREEN_PIN, HIGH);
  digitalWrite(SMOKE_BLUE_PIN, LOW);

  delay(300); // Slight delay for visual effect
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("FireShield 360 - Temperature, Humidity and Smoke Monitoring");

  // Initialize I2C communication
  Wire.begin(21, 22); // SDA on GPIO 21, SCL on GPIO 22
  Serial.println("I2C initialized with SDA=21, SCL=22");

  // Initialize power switch pin - Note: GPIO35 is input-only
  pinMode(POWER_SWITCH_PIN, INPUT_PULLUP);

  // Add a delay to stabilize input reading
  delay(100);

  // Check initial switch position
  int initialReading = digitalRead(POWER_SWITCH_PIN);
  Serial.print("Power switch initialized on GPIO35. Initial state: ");
  Serial.println(initialReading == LOW ? "ON" : "OFF");

  // Initialize OLED display first so we can show progress messages
  Serial.println("Initializing OLED display...");
  if (initDisplay())
  {
    Serial.println("OLED display initialized successfully");
    displaySplashScreen(2000); // Show splash screen for 2 seconds
    displayInitStatus("Display ready", 10);
  }
  else
  {
    Serial.println("Failed to initialize OLED display");
  }

  // Initialize RGB LED for temperature/humidity sensor
  displayInitStatus("Init RGB LEDs", 20);
  initRgbLed(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN);

  // Initialize RGB LED for infrared temperature sensor
  initRgbLed(IR_RED_PIN, IR_GREEN_PIN, IR_BLUE_PIN);

  // Initialize RGB LED for smoke sensor
  initRgbLed(SMOKE_RED_PIN, SMOKE_GREEN_PIN, SMOKE_BLUE_PIN);

  // Show initialization sequence for all RGB LEDs - blue blinking
  displayInitStatus("Testing sensors...", 30);
  showSensorInitializationSequence(SMOKE_RED_PIN, SMOKE_GREEN_PIN, SMOKE_BLUE_PIN, "Smoke sensor");
  showSensorInitializationSequence(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN, "Temp/Hum sensor");
  showSensorInitializationSequence(IR_RED_PIN, IR_GREEN_PIN, IR_BLUE_PIN, "IR Temp sensor");

  // At this point all sensor LEDs are still blue (kept on from showSensorInitializationSequence)

  // Initialize single color LEDs for other indicators
  displayInitStatus("Init status LEDs", 40);
  initLed(WILDFIRE_ALERT_LED_PIN);
  initLed(WIFI_LED_PIN);

  Serial.println("Sensor status LEDs initialized");

  // Initialize smoke sensor (MQ2)
  displayInitStatus("Init MQ2 sensor", 50);
  Serial.println("Initializing MQ2 smoke sensor...");
  initSmokeSensor(SMOKE_PIN);

  // Initialize temperature & humidity sensors (DHT11)
  displayInitStatus("Init DHT11 sensor", 60);
  Serial.println("Initializing DHT11 sensor...");
  initTemperatureSensor(DHT_PIN);
  initHumiditySensor(DHT_PIN);

  // Initialize infrared temperature sensor (MLX90614)
  displayInitStatus("Init IR sensor", 70);
  Serial.println("Initializing MLX90614 infrared sensor...");
  initInfraredSensor();

  // Initialize buzzer
  displayInitStatus("Init buzzer", 65);
  Serial.println("Initializing buzzer...");
  initBuzzer(BUZZER_PIN);

  // Initialize wildfire detection with default thresholds
  displayInitStatus("Init detection logic", 80);
  initWildfireDetection();

  // Initialize WiFi with external LED indicator
  displayInitStatus("Connecting WiFi...", 85);
  initWiFi(ssid, password, WIFI_LED_PIN);

  // Initialize time synchronization
  displayInitStatus("Syncing time...", 90);
  initTimeSync();

  // Initialize MQTT
  displayInitStatus("Connecting MQTT...", 95);
  initMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_password);

  // After MQTT initialization, set the callback function and subscribe to verification topic
  displayInitStatus("MQTT subscriptions", 98);
  setMQTTCallback(mqttCallback);
  subscribeTopic(verification_topic);

  // After all initialization is complete, set all sensor LEDs to green
  displayInitStatus("System ready!", 100);

  // Set all sensor LEDs to green to indicate successful initialization
  setAllSensorsToGreen();

  delay(2000);

  // Show a welcome message on the display
  textDisplay("FireShield 360");

  Serial.println("System initialized and ready.");
}

// Add this global flag to track if we were previously powered off
bool wasPoweredOff = false;

// Add this new function to handle system reinitialization
void reinitializeSystem()
{
  // Initialize OLED display if needed (display should already be on)
  display.ssd1306_command(SSD1306_DISPLAYON);

  // Show splash screen
  displaySplashScreen(2000); // Show splash screen for 2 seconds
  displayInitStatus("Reinitializing...", 10);

  // Initialize RGB LED for temperature/humidity sensor
  displayInitStatus("Init RGB LEDs", 20);
  initRgbLed(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN);

  // Initialize RGB LED for infrared temperature sensor
  initRgbLed(IR_RED_PIN, IR_GREEN_PIN, IR_BLUE_PIN);

  // Initialize RGB LED for smoke sensor
  initRgbLed(SMOKE_RED_PIN, SMOKE_GREEN_PIN, SMOKE_BLUE_PIN);

  // Show initialization sequence for all RGB LEDs - blue blinking
  displayInitStatus("Testing sensors...", 30);
  showSensorInitializationSequence(SMOKE_RED_PIN, SMOKE_GREEN_PIN, SMOKE_BLUE_PIN, "Smoke sensor");
  showSensorInitializationSequence(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN, "Temp/Hum sensor");
  showSensorInitializationSequence(IR_RED_PIN, IR_GREEN_PIN, IR_BLUE_PIN, "IR Temp sensor");

  // Initialize single color LEDs for other indicators
  displayInitStatus("Init status LEDs", 40);
  initLed(WILDFIRE_ALERT_LED_PIN);
  initLed(WIFI_LED_PIN);

  Serial.println("Sensor status LEDs reinitialized");

  // Reinitialize sensors (don't need to change pin config, just ensure they're working)
  displayInitStatus("Checking sensors", 50);
  // Temperature and humidity sensors
  // Smoke sensor
  // Infrared temperature sensor

  // Initialize WiFi with external LED indicator
  displayInitStatus("Reconnecting WiFi...", 70);
  initWiFi(ssid, password, WIFI_LED_PIN);

  // Initialize time synchronization
  displayInitStatus("Syncing time...", 80);
  initTimeSync();

  // Initialize MQTT
  displayInitStatus("Reconnecting MQTT...", 90);
  reconnectMQTT();

  // Reinitialize MQTT subscriptions
  displayInitStatus("MQTT subscriptions", 95);
  reinitializeMQTTSubscriptions();

  // After all initialization is complete, set all sensor LEDs to green
  displayInitStatus("System ready!", 100);

  // Set all sensor LEDs to green to indicate successful initialization
  setAllSensorsToGreen();

  delay(2000);

  // Show a welcome message on the display
  textDisplay("FireShield 360");

  Serial.println("System reinitialized and ready.");
}

// Callback function for MQTT messages
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // Convert the payload to a string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Message: ");
  Serial.println(message);

  // Check if this is a verification message
  if (strcmp(topic, verification_topic) == 0)
  {
    // Parse the JSON message
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract the verified wildfire detection status
    bool verifiedWildfire = doc["wildfire_detected"];

    Serial.print("VERIFICATION STATUS RECEIVED: Wildfire detected = ");
    Serial.println(verifiedWildfire ? "TRUE" : "FALSE");

    if (verifiedWildfire)
    {
      Serial.println("VERIFIED WILDFIRE DETECTED BY BACKEND!");

      // Display verification message
      textDisplay("VERIFIED WILDFIRE!");

      // Sound the verified alert tone for 10 seconds
      soundVerifiedAlert(BUZZER_ALERT_DURATION);

      // Store the verification status
      verifiedWildfireDetected = true;
    }
    else
    {
      Serial.println("Backend verification: No wildfire detected");

      // Reset the verification status
      verifiedWildfireDetected = false;
    }
  }
}

// Add this function to reinitialize MQTT subscriptions
void reinitializeMQTTSubscriptions()
{
  // Set the callback and resubscribe to the verification topic
  setMQTTCallback(mqttCallback);
  subscribeTopic(verification_topic);
}

void loop()
{
  // Check power switch state with debouncing
  bool powerOn = isPowerOn();

  if (!powerOn)
  {
    // Only perform power down sequence if this is the first time detecting power off
    if (!wasPoweredOff)
    {
      Serial.println("Power switch turned OFF - powering down device");

      // Disconnect from MQTT and WiFi
      disconnectMQTT();
      disconnectWiFi();

      // Device is switched off - turn off all LEDs
      digitalWrite(TEMP_HUM_RED_PIN, LOW);
      digitalWrite(TEMP_HUM_GREEN_PIN, LOW);
      digitalWrite(TEMP_HUM_BLUE_PIN, LOW);

      digitalWrite(IR_RED_PIN, LOW);
      digitalWrite(IR_GREEN_PIN, LOW);
      digitalWrite(IR_BLUE_PIN, LOW);

      digitalWrite(SMOKE_RED_PIN, LOW);
      digitalWrite(SMOKE_GREEN_PIN, LOW);
      digitalWrite(SMOKE_BLUE_PIN, LOW);

      turnOffLed(WILDFIRE_ALERT_LED_PIN);
      turnOffLed(WIFI_LED_PIN);

      // Display power off message
      displayPowerOff();

      // Wait 3 seconds to show the message
      delay(3000);

      // Turn off the display to save power
      clearDisplay();
      display.ssd1306_command(SSD1306_DISPLAYOFF);

      wasPoweredOff = true;
    }

    // Add more delay in power-off state to reduce chance of false triggers
    delay(200);
    return;
  }

  // If we're here, power is back on
  if (wasPoweredOff)
  {
    // Add a safety delay before reinitializing to avoid rapid power cycling
    delay(500);

    // Double-check power is still on after delay
    if (isPowerOn())
    {
      Serial.println("Power switch turned ON - powering up device");
      reinitializeSystem();
      wasPoweredOff = false;
    }
  }

  // Continue with normal operation when powered on
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
  bool tempHumSensorStatus = updateSensorStatusLed(TEMP_HUM_RED_PIN, TEMP_HUM_GREEN_PIN, TEMP_HUM_BLUE_PIN, tempHumValid);

  // Debug info for DHT11 errors
  if (!tempHumSensorStatus)
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

  // Update infrared temperature sensor status with RGB LED
  bool irValid = isInfraredReadingValid();
  bool irSensorStatus = updateSensorStatusLed(IR_RED_PIN, IR_GREEN_PIN, IR_BLUE_PIN, irValid);

  // Debug info for infrared sensor errors
  if (!irSensorStatus)
  {
    Serial.println("IR ERROR: Infrared temperature sensor disconnected or failed");
  }

  // Update smoke sensor status with RGB LED
  bool smokeValid = isSmokeReadingValid();
  bool smokeSensorStatus = updateSensorStatusLed(SMOKE_RED_PIN, SMOKE_GREEN_PIN, SMOKE_BLUE_PIN, smokeValid);

  // Debug info for smoke sensor errors
  if (!smokeSensorStatus)
  {
    Serial.println("SMOKE ERROR: Smoke sensor disconnected or failed");
  }

  // Process MQTT messages
  processMQTTMessages();

  // Check for wildfire conditions - now requires at least 2 sensors exceeding thresholds
  int thresholdsExceeded = getNumberOfThresholdsExceeded(temperature, humidity, smokeValue, irTemperature);
  wildfireDetected = (thresholdsExceeded >= 2); // Alert when 2 or more thresholds are exceeded

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

      // Use the main topic instead of the dedicated wildfire topic
      if (publishWildfireAlert(temperature, humidity, smokeValue, irTemperature,
                               thresholdsExceeded,
                               mqtt_topic)) // Using main topic here
      {
        Serial.println("Wildfire alert published to MQTT");
        Serial.print("Thresholds exceeded: ");
        Serial.println(thresholdsExceeded);

        // Activate the buzzer for local fire alerts
        if (!isBuzzerActive())
        {
          Serial.println("POTENTIAL WILDFIRE DETECTED - activating local alert buzzer");
          soundFireAlert(BUZZER_ALERT_DURATION);
        }
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

    // Always call displaySensorData, passing validity status for each sensor
    bool tempValid = isTemperatureReadingValid();
    bool humidityValid = isHumidityReadingValid();
    bool smokeValid = isSmokeReadingValid();
    bool irValid = isInfraredReadingValid();

    displaySensorData(temperature, humidity, smokeValue, irTemperature,
                      wildfireDetected,
                      tempValid, humidityValid, smokeValid, irValid);
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

  // Update the buzzer (controls on/off timing for alerts)
  updateBuzzer();

  // Wait before next reading
  delay(1000);
}
