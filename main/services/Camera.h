#ifndef ESP32_REMOTECAMERA_CAMERA_H
#define ESP32_REMOTECAMERA_CAMERA_H

#include "Utils.h"
#include <esp_err.h>

extern esp_err_t camera_init();

extern void camera_read();

#endif //ESP32_REMOTECAMERA_CAMERA_H
