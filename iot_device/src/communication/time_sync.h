#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <time.h>

// Initialize NTP time sync
void initTimeSync();

// Sync time with NTP servers
bool syncTime();

// Get current time as ISO8601 formatted string (2024-05-19T10:15:30Z)
void getFormattedTime(char *buffer, size_t bufferSize);

// Check if time is synchronized
bool isTimeSynchronized();

#endif // TIME_SYNC_H
