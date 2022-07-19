#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGENVS_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGENVS_H

#include <stdbool.h>
#include <esp_err.h>

extern StorageError nvs_init();

extern StorageError nvs_destroy();

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGENVS_H
