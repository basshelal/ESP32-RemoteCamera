#ifndef ESP32_REMOTECAMERA_DATASTORE_H
#define ESP32_REMOTECAMERA_DATASTORE_H

#include "Utils.h"
#include <esp_err.h>

extern struct {
    esp_err_t (*init)();
} DataStore;

extern esp_err_t dataStore_init();

#endif //ESP32_REMOTECAMERA_DATASTORE_H
