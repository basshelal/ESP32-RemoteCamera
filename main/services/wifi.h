#ifndef ESP32_REMOTECAMERA_WIFI_H
#define ESP32_REMOTECAMERA_WIFI_H

#include <esp_err.h>
#include <esp_wifi_types.h>

typedef wifi_mode_t WifiMode;

extern esp_err_t wifi_init();

extern esp_err_t wifi_start(WifiMode wifiMode);

extern WifiMode wifi_getCurrentMode();

extern esp_err_t wifi_stop();

#endif //ESP32_REMOTECAMERA_WIFI_H
