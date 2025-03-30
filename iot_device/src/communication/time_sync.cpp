#include <Arduino.h>
#include "time_sync.h"
#include "wifi.h"

// NTP Server settings
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";

// Time zone settings (modify these for your local time)
const long gmtOffset_sec = 19800; // UTC+5:30 for Sri Lanka (5 hours * 3600 + 30 minutes * 60)
const int daylightOffset_sec = 0; // No DST in Sri Lanka
bool timeIsSynchronized = false;

void initTimeSync()
{
    Serial.println("Initializing time service with local time offset...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    syncTime();
}

bool syncTime()
{
    if (!isWiFiConnected())
    {
        Serial.println("Cannot sync time - WiFi not connected");
        timeIsSynchronized = false;
        return false;
    }

    time_t now = time(nullptr);
    int attempts = 0;
    while (now < 24 * 3600 && attempts < 10)
    {
        Serial.println("Waiting for NTP time sync...");
        delay(500);
        now = time(nullptr);
        attempts++;
    }

    if (now > 24 * 3600)
    {
        struct tm timeinfo;
        gmtime_r(&now, &timeinfo);
        Serial.print("Time synchronized: ");
        Serial.print(asctime(&timeinfo));
        timeIsSynchronized = true;
        return true;
    }
    else
    {
        Serial.println("Failed to sync time");
        timeIsSynchronized = false;
        return false;
    }
}

void getFormattedTime(char *buffer, size_t bufferSize)
{
    time_t now;
    struct tm timeinfo;

    if (timeIsSynchronized && buffer != NULL && bufferSize >= 25)
    {
        time(&now);
        localtime_r(&now, &timeinfo); // Using localtime_r instead of gmtime_r to get local time

        // Format as ISO8601 with local time: YYYY-MM-DDThh:mm:ss+HH:MM
        // Calculate timezone offset string
        int tzHours = gmtOffset_sec / 3600;
        int tzMinutes = (gmtOffset_sec % 3600) / 60;

        char tzSign = '+';
        if (tzHours < 0)
        {
            tzSign = '-';
            tzHours = abs(tzHours);
            tzMinutes = abs(tzMinutes);
        }

        snprintf(buffer, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec,
                 tzSign,
                 tzHours,
                 tzMinutes);
    }
    else if (buffer != NULL && bufferSize >= 25)
    {
        // If time is not synchronized, provide a fallback
        snprintf(buffer, bufferSize, "1970-01-01T00:00:00+00:00");
    }
}

bool isTimeSynchronized()
{
    return timeIsSynchronized;
}
