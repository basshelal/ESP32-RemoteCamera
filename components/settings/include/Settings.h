#ifndef ESP32_REMOTECAMERA_SETTINGS_H
#define ESP32_REMOTECAMERA_SETTINGS_H

#include "Utils.h"
#include <stddef.h>
#include <stdbool.h>

#define SETTINGS_KEY_WIFI_SSID "wifissid"
#define SETTINGS_KEY_WIFI_PASSWORD "wifipassword"
#define SETTINGS_KEY_WIFI_IP_ADDRESS "wifiipaddress"

typedef char SettingsKey;

typedef enum SettingsError {
    SETTINGS_ERROR_NONE = 0,
    SETTINGS_ERROR_INVALID_PARAMETER,
    SETTINGS_ERROR_NOT_FOUND,
    SETTINGS_ERROR_PARTITION_NOT_FOUND,
    SETTINGS_ERROR_KEY_NOT_FOUND,
    SETTINGS_ERROR_INVALID_KEY,
    SETTINGS_ERROR_INVALID_VALUE,
    SETTINGS_ERROR_NVS_CLOSED,
    SETTINGS_ERROR_NOT_ENOUGH_SPACE,
    SETTINGS_ERROR_INVALID_LENGTH,
    SETTINGS_ERROR_ALREADY_EXISTS,
    SETTINGS_ERROR_GENERIC_FAILURE = -1,
} SettingsError;

extern SettingsError settings_init();

extern SettingsError settings_destroy();

/** Puts the value at key, updating it if it exists and creating it if it doesn't,
 * you can check a key's existence using \c settings_hasKey */
extern SettingsError settings_putString(const SettingsKey *key, const char *value);

extern SettingsError settings_getString(const SettingsKey *key, char *value);

extern SettingsError settings_getStringLength(const SettingsKey *key, size_t *length);

extern SettingsError settings_putUInt32(const SettingsKey *key, const uint32_t value);

extern SettingsError settings_getUInt32(const SettingsKey *key, uint32_t *value);

extern SettingsError settings_deleteKey(const SettingsKey *key);

extern SettingsError settings_hasKey(const SettingsKey *key, bool *hasKey);

#endif //ESP32_REMOTECAMERA_SETTINGS_H
