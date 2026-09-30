#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include "config.h"

#ifdef PLATFORM_ESP8266
    #include <LittleFS.h>
    #define FILESYSTEM LittleFS
#else
    #include <SPIFFS.h>
    #define FILESYSTEM SPIFFS
#endif

// Mount the filesystem (LittleFS on ESP8266, SPIFFS on ESP32). Safe to call
// more than once. Never blocks startup: an empty or damaged partition, or one
// left behind by an unfinished image update, is formatted so the device always
// boots with a usable (possibly empty) filesystem.
bool fsBegin();

bool fsMounted();

// True when the web interface files (index, script, style) are present
bool fsHasWebUi();

// Why fsBegin() had to erase or format the filesystem, or nullptr. Logged
// by the logger once it is up (fsBegin() runs before logging is available).
const char* fsResetReason();

#ifndef PLATFORM_ESP8266
// Call before writing a filesystem image into the partition: unmounts the
// filesystem and records the update in NVS, so a power loss mid-update is
// detected at the next boot.
void fsUpdateStart();

// Call when the image update ends. On failure, a partition that already
// received data is formatted (it holds a mix of old and new image) and the
// filesystem is mounted again, so logging keeps working and the recovery
// page is shown instead of broken files.
void fsUpdateEnd(bool success, size_t bytesWritten);
#endif

#endif // FILESYSTEM_H
