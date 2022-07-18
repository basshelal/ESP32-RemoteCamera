#ifndef ESP32_REMOTECAMERA_BATTERY_H
#define ESP32_REMOTECAMERA_BATTERY_H

#include "Utils.h"
#include <esp_err.h>

extern esp_err_t battery_init();

extern uint32_t battery_raw(const int samplesCount);

extern float battery_percentage();

extern float battery_voltage();

extern const char *battery_text();

#endif //ESP32_REMOTECAMERA_BATTERY_H
