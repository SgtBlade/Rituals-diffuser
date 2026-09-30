#include "ota_handler.h"
#include "config.h"
#include "storage.h"
#include "filesystem.h"

OTAHandler otaHandler;

#ifdef PLATFORM_ESP8266

// ESP8266: ArduinoOTA disabled to save RAM (~1KB+)
// Use web-based Safe Update mode instead (sync_ota.cpp)
void OTAHandler::begin() {}
void OTAHandler::loop() {}
void OTAHandler::onProgress(OTACallback callback) { _progressCallback = callback; }
void OTAHandler::onStart(void (*callback)()) { _startCallback = callback; }
void OTAHandler::onEnd(void (*callback)()) { _endCallback = callback; }
void OTAHandler::onError(void (*callback)(int error)) { _errorCallback = callback; }

#else

#include <ArduinoOTA.h>

void OTAHandler::begin() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(storage.getOTAPassword());

    ArduinoOTA.onStart([this]() {
        _active = true;
        _fsUpdate = (ArduinoOTA.getCommand() != U_FLASH);
        _bytesReceived = 0;
        Serial.printf("[OTA] Start updating %s\n", _fsUpdate ? "filesystem" : "firmware");

        if (_fsUpdate) {
            // The partition is being overwritten: unmount it and record the
            // update, so an interrupted upload is detected at the next boot
            fsUpdateStart();
        }

        if (_startCallback) {
            _startCallback();
        }
    });

    ArduinoOTA.onEnd([this]() {
        Serial.println("\n[OTA] Update complete");
        _active = false;
        if (_fsUpdate) {
            fsUpdateEnd(true, _bytesReceived);
            _fsUpdate = false;
        }

        if (_endCallback) {
            _endCallback();
        }
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        _bytesReceived = progress;
        int percent = (total > 0) ? ((progress * 100) / total) : 0;
        Serial.printf("[OTA] Progress: %u%%\r", percent);

        if (_progressCallback) {
            _progressCallback(percent);
        }
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");

        // Errors before onStart (auth, begin failed because another update
        // is running) must not touch that other update's state. After a
        // connect error ArduinoOTA also reports an end error: handle once.
        if (!_active) return;
        _active = false;

        if (_fsUpdate) {
            _fsUpdate = false;
            fsUpdateEnd(false, _bytesReceived);
        }
        if (_errorCallback) {
            _errorCallback((int)error);
        }
    });

    ArduinoOTA.begin();
    Serial.println("[OTA] Service started");
}

void OTAHandler::loop() {
    ArduinoOTA.handle();
}

void OTAHandler::onProgress(OTACallback callback) {
    _progressCallback = callback;
}

void OTAHandler::onStart(void (*callback)()) {
    _startCallback = callback;
}

void OTAHandler::onEnd(void (*callback)()) {
    _endCallback = callback;
}

void OTAHandler::onError(void (*callback)(int error)) {
    _errorCallback = callback;
}

#endif
