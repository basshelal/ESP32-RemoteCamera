#ifndef ESP32_REMOTECAMERA_CAMERA_H
#define ESP32_REMOTECAMERA_CAMERA_H

#include "Error.h"

extern Error camera_init();

extern Error camera_destroy();

extern void camera_read();

#endif //ESP32_REMOTECAMERA_CAMERA_H
