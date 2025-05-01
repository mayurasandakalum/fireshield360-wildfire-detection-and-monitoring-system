#include "buzzer.h"

// Buzzer pin
static int buzzerPin = -1;

// Buzzer timing variables
static unsigned long alertStartTime = 0;
static unsigned long alertDuration = 0;
static bool isAlertActive = false;

// Alert type
enum AlertType
{
    NONE,
    FIRE_ALERT,
    VERIFIED_ALERT
};

static AlertType currentAlertType = NONE;

// Fire alert pattern timing
static const int fireAlertHighTime = 200; // 200ms on
static const int fireAlertLowTime = 100;  // 100ms off

// Verified alert pattern timing
static const int verifiedAlertHighTime = 500; // 500ms on
static const int verifiedAlertLowTime = 500;  // 500ms off

// Tone frequencies
static const int fireAlertTone = 1000;     // 1kHz tone for fire alert
static const int verifiedAlertTone = 1000; // 1kHz tone for verified alert

// PWM channel for tone generation
static const int pwmChannel = 0;
static const int pwmResolution = 8;
static const int pwmDutyCycle = 127; // 50% duty cycle

// Function to generate a tone using PWM (ESP32 doesn't have built-in tone() function)
void generateTone(int pin, int frequency)
{
    if (frequency == 0)
    {
        // No tone - equivalent to noTone()
        ledcDetachPin(pin);
        digitalWrite(pin, LOW);
    }
    else
    {
        // Generate a tone using PWM
        ledcSetup(pwmChannel, frequency, pwmResolution);
        ledcAttachPin(pin, pwmChannel);
        ledcWrite(pwmChannel, pwmDutyCycle);
    }
}

void stopTone(int pin)
{
    // Detach the PWM channel from the pin
    ledcDetachPin(pin);
    digitalWrite(pin, LOW);
}

void initBuzzer(int pin)
{
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW); // Ensure buzzer is off at start
    Serial.print("Buzzer initialized on pin ");
    Serial.println(buzzerPin);
}

void soundFireAlert(unsigned long duration)
{
    if (buzzerPin == -1)
    {
        Serial.println("ERROR: Buzzer not initialized");
        return;
    }

    alertStartTime = millis();
    alertDuration = duration;
    isAlertActive = true;
    currentAlertType = FIRE_ALERT;

    Serial.print("Fire alert sounding for ");
    Serial.print(duration / 1000);
    Serial.println(" seconds");
}

void soundVerifiedAlert(unsigned long duration)
{
    if (buzzerPin == -1)
    {
        Serial.println("ERROR: Buzzer not initialized");
        return;
    }

    alertStartTime = millis();
    alertDuration = duration;
    isAlertActive = true;
    currentAlertType = VERIFIED_ALERT;

    Serial.print("Verified alert tone sounding for ");
    Serial.print(duration / 1000);
    Serial.println(" seconds");
}

void stopBuzzer()
{
    if (buzzerPin == -1)
    {
        return;
    }

    stopTone(buzzerPin);
    isAlertActive = false;
    currentAlertType = NONE;
}

bool isBuzzerActive()
{
    return isAlertActive;
}

void updateBuzzer()
{
    if (!isAlertActive || buzzerPin == -1)
    {
        return;
    }

    unsigned long currentTime = millis();

    // Check if alert duration has passed
    if (currentTime - alertStartTime >= alertDuration)
    {
        stopBuzzer();
        return;
    }

    if (currentAlertType == FIRE_ALERT)
    {
        // Generate fire alert pattern with tone
        unsigned long elapsedTime = (currentTime - alertStartTime) % (fireAlertHighTime + fireAlertLowTime);

        if (elapsedTime < fireAlertHighTime)
        {
            generateTone(buzzerPin, fireAlertTone); // Use 1kHz tone instead of just HIGH
        }
        else
        {
            stopTone(buzzerPin); // Properly stop the tone
        }
    }
    else if (currentAlertType == VERIFIED_ALERT)
    {
        // Generate verified alert pattern with tone
        unsigned long elapsedTime = (currentTime - alertStartTime) % (verifiedAlertHighTime + verifiedAlertLowTime);

        if (elapsedTime < verifiedAlertHighTime)
        {
            generateTone(buzzerPin, verifiedAlertTone);
        }
        else
        {
            stopTone(buzzerPin);
        }
    }
}
