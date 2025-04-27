it21243226: in there, assume i connected the usb cable to esp32. if i attatch a switch to the esp32, using that can i on or off the device with other sensors. (switch is not directly connected to the power)

GitHub Copilot: # Adding Power Switch Functionality to ESP32

You can definitely add a switch to control the device state without directly connecting it to the power supply. This is a good approach for a clean software shutdown/startup. Here's how to implement it:

## 1. Hardware Connection

Connect a simple toggle or push button switch:

- Connect one terminal to a GPIO pin (I'll use GPIO18)
- Connect the other terminal to GND
- Use the internal pull-up resistor in the ESP32

## 2. Code Implementation

First, add the necessary definitions:

```cpp
// GPIO pin for power switch
const int POWER_SWITCH_PIN = 18;  // GPIO18 for power switch
bool devicePoweredOn = true;     // Track device power state
```

Then modify the setup() function to initialize the switch:

```cpp
// In setup() function, after initializing other pins:
pinMode(POWER_SWITCH_PIN, INPUT_PULLUP);  // Configure with internal pull-up
Serial.println("Power switch initialized on GPIO18");
```

Finally, add power management logic at the beginning of your loop() function:

```cpp
void loop() {
  // Check power switch state (LOW when switch is closed/on)
  bool switchPressed = (digitalRead(POWER_SWITCH_PIN) == LOW);

  // Simple debounce and state change detection
  static bool lastSwitchState = false;
  static unsigned long lastDebounceTime = 0;

  if (switchPressed != lastSwitchState) {
    lastDebounceTime = millis();
  }

  // If switch state has been stable for 50ms, process it
  if ((millis() - lastDebounceTime) > 50) {
    // If switch state changed from OFF to ON (button press complete)
    if (switchPressed && !lastSwitchState) {
      devicePoweredOn = !devicePoweredOn;  // Toggle power state

      if (devicePoweredOn) {
        // Power on sequence
        Serial.println("Device powering ON");
        textDisplay("Powering ON...");
        setAllSensorsToGreen();  // Visual feedback
        delay(1000);
        textDisplay("FireShield 360");
      } else {
        // Power off sequence
        Serial.println("Device powering OFF");
        textDisplay("Powering OFF...");

        // Turn off all indicator LEDs
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
        delay(1000);
        textDisplay("System OFF");
      }
    }
  }

  lastSwitchState = switchPressed;

  // Only process sensors and main functionality if device is powered on
  if (!devicePoweredOn) {
    delay(100);  // Small delay to prevent CPU hogging while "off"
    return;      // Skip the rest of the loop
  }

  // Original loop code continues below...
  // Check WiFi status and update LED
  ensureWiFiConnected();

  // ... rest of your original loop() code
}
```

This implementation:

1. Uses a GPIO pin with pull-up resistor to detect switch state
2. Implements basic debouncing to prevent false triggers
3. Toggles between "on" and "off" states when the switch is pressed
4. Shows power status on the display
5. Turns off all LEDs when powering down
6. Skips all sensor readings and processing when in the "off" state

For better power efficiency, you could also add deep sleep functionality when powered off, but that's more complex and would require additional changes to the wake-up logic.
