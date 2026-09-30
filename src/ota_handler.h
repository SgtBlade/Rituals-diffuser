#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include "config.h"

class OTAHandler {
public:
    void begin();
    void loop();

    // Callback for OTA events
    typedef void (*OTACallback)(int progress);
    void onProgress(OTACallback callback);
    void onStart(void (*callback)());
    void onEnd(void (*callback)());
    void onError(void (*callback)(int error));

private:
    OTACallback _progressCallback = nullptr;
    void (*_startCallback)() = nullptr;
    void (*_endCallback)() = nullptr;
    void (*_errorCallback)(int error) = nullptr;
    bool _active = false;          // An ArduinoOTA upload passed onStart
    bool _fsUpdate = false;        // Current ArduinoOTA upload is a filesystem image
    size_t _bytesReceived = 0;
};

extern OTAHandler otaHandler;

#endif // OTA_HANDLER_H
