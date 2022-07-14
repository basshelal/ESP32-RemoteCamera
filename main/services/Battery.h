#ifndef ESP32_REMOTECAMERA_BATTERY_H
#define ESP32_REMOTECAMERA_BATTERY_H

#include <esp_err.h>
#include "utils.h"

extern esp_err_t battery_init();

extern esp_err_t battery_read();

#endif //ESP32_REMOTECAMERA_BATTERY_H
