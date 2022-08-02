#ifndef ESP32_REMOTECAMERA_WEBSERVER_H
#define ESP32_REMOTECAMERA_WEBSERVER_H

#include "Utils.h"
#include "WebserverError.h"

extern WebserverError webserver_init();

extern WebserverError webserver_destroy();

extern WebserverError webserver_start();

#endif //ESP32_REMOTECAMERA_WEBSERVER_H
