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
static const int verifiedAlertTone = 1000;    // 1kHz tone

// Function to generate a tone using PWM (ESP32 doesn't have built-in tone() function)
void generateTone(int pin, int frequency)
{
    if (frequency == 0)
    {
        // No tone - equivalent to noTone()
        digitalWrite(pin, LOW);
    }
    else
    {
        // Generate a tone using PWM
        // ESP32 ledcWrite approach
        // For simplicity in this implementation, we'll use a 50% duty cycle
        int pwmChannel = 0;
        int resolution = 8;
        int dutyCycle = 127; // 50% of 255

        ledcSetup(pwmChannel, frequency, resolution);
        ledcAttachPin(pin, pwmChannel);
        ledcWrite(pwmChannel, dutyCycle);
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

    if (currentAlertType == VERIFIED_ALERT)
    {
        stopTone(buzzerPin);
    }
    else
    {
        digitalWrite(buzzerPin, LOW);
    }

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
        // Generate fire alert pattern
        unsigned long elapsedTime = (currentTime - alertStartTime) % (fireAlertHighTime + fireAlertLowTime);

        if (elapsedTime < fireAlertHighTime)
        {
            digitalWrite(buzzerPin, HIGH); // Buzzer on
        }
        else
        {
            digitalWrite(buzzerPin, LOW); // Buzzer off
        }
    }
    else if (currentAlertType == VERIFIED_ALERT)
    {
        // Generate verified alert pattern with tone
        unsigned long elapsedTime = (currentTime - alertStartTime) % (verifiedAlertHighTime + verifiedAlertLowTime);

        if (elapsedTime < verifiedAlertHighTime)
        {
            generateTone(buzzerPin, verifiedAlertTone); // Buzzer on with tone
        }
        else
        {
            stopTone(buzzerPin); // Buzzer off
        }
    }
}
