#include "filesystem.h"

#ifndef PLATFORM_ESP8266
    #include <Preferences.h>
    #include <esp_partition.h>
#endif

static bool _mounted = false;
static bool _webUi = false;
static const char* _resetReason = nullptr;

static bool checkWebUi() {
    static const char* const files[] = { "/index.html", "/script.js", "/style.css" };
    for (const char* f : files) {
        String gz = String(f) + ".gz";
        if (!FILESYSTEM.exists(gz) && !FILESYSTEM.exists(f)) {
            return false;
        }
    }
    return true;
}

#ifndef PLATFORM_ESP8266
// NVS flag set while a filesystem image is being written
#define FS_NVS_NAMESPACE "fs"
#define FS_NVS_UPDATE_KEY "update"

static bool updateMarker() {
    Preferences prefs;
    if (!prefs.begin(FS_NVS_NAMESPACE, true)) return false;  // Namespace absent: no marker
    bool set = prefs.getBool(FS_NVS_UPDATE_KEY, false);
    prefs.end();
    return set;
}

static void setUpdateMarker(bool set) {
    Preferences prefs;
    if (!prefs.begin(FS_NVS_NAMESPACE, false)) return;
    if (set) {
        prefs.putBool(FS_NVS_UPDATE_KEY, true);
    } else {
        prefs.remove(FS_NVS_UPDATE_KEY);
    }
    prefs.end();
}

// Erase the whole partition. SPIFFS then sees blank flash and formats it on
// mount, without ever parsing the damaged contents.
static void erasePartition() {
    const esp_partition_t* part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, nullptr);
    if (part == nullptr) {
        Serial.println("[FS] No SPIFFS partition found");
        return;
    }
    esp_err_t err = esp_partition_erase_range(part, 0, part->size);
    Serial.printf("[FS] Partition erased (%u bytes): %s\n", (unsigned)part->size,
                  err == ESP_OK ? "OK" : esp_err_to_name(err));
}
#endif

bool fsBegin() {
    if (_mounted) return true;

#ifdef PLATFORM_ESP8266
    // LittleFS formats automatically when the mount fails
    _mounted = FILESYSTEM.begin();
#else
    if (updateMarker()) {
        // A filesystem update did not finish (power loss or reset while
        // writing): the partition holds part of the new image and part of the
        // old one. Don't try to mount that.
        Serial.println("[FS] Unfinished filesystem update detected, erasing partition");
        erasePartition();
        setUpdateMarker(false);
        _resetReason = "Filesystem erased: previous filesystem update did not finish";
    }

    _mounted = FILESYSTEM.begin(false);
    if (!_mounted) {
        Serial.println("[FS] Mount failed, formatting");
        if (_resetReason == nullptr) _resetReason = "Filesystem formatted: mount failed";
        _mounted = FILESYSTEM.format() && FILESYSTEM.begin(false);
    }

    // A mount that succeeds on garbage can report impossible sizes
    if (_mounted && FILESYSTEM.usedBytes() > FILESYSTEM.totalBytes()) {
        Serial.println("[FS] Filesystem inconsistent, formatting");
        _resetReason = "Filesystem formatted: inconsistent";
        FILESYSTEM.end();
        erasePartition();
        _mounted = FILESYSTEM.begin(true);
    }
#endif

    if (!_mounted) {
        Serial.println("[FS] Filesystem unavailable, continuing without it");
        _webUi = false;
        return false;
    }

    _webUi = checkWebUi();
#ifdef PLATFORM_ESP8266
    Serial.printf("[FS] Mounted, web interface %s\n", _webUi ? "present" : "MISSING");
#else
    Serial.printf("[FS] Mounted (%u/%u bytes used), web interface %s\n",
                  (unsigned)FILESYSTEM.usedBytes(), (unsigned)FILESYSTEM.totalBytes(),
                  _webUi ? "present" : "MISSING");
#endif
    return true;
}

const char* fsResetReason() {
    return _resetReason;
}

bool fsMounted() {
    return _mounted;
}

bool fsHasWebUi() {
    return _mounted && _webUi;
}

#ifndef PLATFORM_ESP8266
void fsUpdateStart() {
    _webUi = false;
    if (_mounted) {
        FILESYSTEM.end();
        _mounted = false;
    }
    setUpdateMarker(true);
}

void fsUpdateEnd(bool success, size_t bytesWritten) {
    if (success) {
        // A restart follows; the new image is mounted at boot
        setUpdateMarker(false);
        return;
    }

    if (bytesWritten > 0) {
        Serial.println("[FS] Filesystem update failed, partition is incomplete");
        erasePartition();
    }
    setUpdateMarker(false);
    fsBegin();
}
#endif
