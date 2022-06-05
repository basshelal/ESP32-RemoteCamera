#ifndef ESP32_REMOTECAMERA_WEBSERVER_H
#define ESP32_REMOTECAMERA_WEBSERVER_H

#include <esp_err.h>

extern struct {
    esp_err_t (*init)();
} WebServer;

extern esp_err_t webServer_init();

#endif //ESP32_REMOTECAMERA_WEBSERVER_H