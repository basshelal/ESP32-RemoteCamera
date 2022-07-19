#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGESPIFFS_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGESPIFFS_H

#include "StorageError.h"
#include <esp_err.h>

extern StorageError spiffs_init();

extern StorageError spiffs_destroy();

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGESPIFFS_H
