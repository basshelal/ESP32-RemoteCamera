#ifndef ESP32_REMOTECAMERA_WEBSERVER_H
#define ESP32_REMOTECAMERA_WEBSERVER_H

#include "Error.h"
#include "Utils.h"

extern Error webserver_init();

extern Error webserver_destroy();

extern Error webserver_start();

#endif //ESP32_REMOTECAMERA_WEBSERVER_H
