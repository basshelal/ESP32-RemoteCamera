#include "Settings.h"
#include "Utils.h"
#include "Logger.h"
#include <esp_err.h>
#include <nvs_flash.h>
#include <string.h>

#define NVS_PARTITION NVS_DEFAULT_PART_NAME
#define NVS_NAMESPACE "default"

private nvs_handle_t nvsDefaultHandle;

public SettingsError settings_init() {
    VERBOSE("Initializing NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NOT_FOUND) {
        throw(SETTINGS_ERROR_PARTITION_NOT_FOUND, "No partition with label \"%s\", cannot proceed!", NVS_PARTITION);
    }
    if (err == ESP_ERR_NVS_NO_FREE_PAGES //  NVS storage contains no empty pages (partition was truncated)
        || err == ESP_ERR_NVS_NEW_VERSION_FOUND) { // NVS partition contains data in new format
        VERBOSE("nvs_flash_init() failed, erasing and retrying");
        err = nvs_flash_erase();
        if (err == ESP_ERR_NOT_FOUND) {
            throw(SETTINGS_ERROR_PARTITION_NOT_FOUND, "No partition with label \"%s\", cannot proceed!",
                  NVS_PARTITION);
        } else {
            err = nvs_flash_init();
            if (err != ESP_OK) {
                throw(SETTINGS_ERROR_GENERIC_FAILURE, "NVS init failed after retry, error: %s", esp_err_to_name(err));
            }
            VERBOSE("nvs_flash_init() retry successful");
        }
    }
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvsDefaultHandle);
    if (err == ESP_ERR_NVS_PART_NOT_FOUND) {
        throw(SETTINGS_ERROR_PARTITION_NOT_FOUND, "No partition with label \"%s\", cannot proceed!", NVS_PARTITION);
    } else if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "NVS driver was not initialized");
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_open() returned error: %s", esp_err_to_name(err));
    }
    VERBOSE("Successfully initialized NVS");
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_destroy() {
    VERBOSE("Destroying NVS");
    nvs_close(nvsDefaultHandle);
    esp_err_t err = nvs_flash_deinit();
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        WARN("nvs_flash_deinit() returned: %s, NVS was not initialized", esp_err_to_name(err));
    }
    VERBOSE("Successfully destroyed NVS");
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_putString(const SettingsKey *key, const char *value) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    requireNotNull(value, SETTINGS_ERROR_INVALID_PARAMETER, "value cannot be a NULL pointer");
    esp_err_t err = nvs_set_str(nvsDefaultHandle, key, value);
    if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(SETTINGS_ERROR_NVS_CLOSED, "NVS handled is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(SETTINGS_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        throw(SETTINGS_ERROR_NOT_ENOUGH_SPACE, "Not enough space in NVS to save key: %s, value: %s",
              key, value);
    } else if (err == ESP_ERR_NVS_VALUE_TOO_LONG) {
        throw(SETTINGS_ERROR_INVALID_VALUE,
              "Value too long, max size is 4000 bytes at best (including terminator), value: %s", value);
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_set_str() returned %s", esp_err_to_name(err));
    }
    err = nvs_commit(nvsDefaultHandle);
    if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_commit() returned %s", esp_err_to_name(err));
    }
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_getStringLength(const SettingsKey *key,
                                              size_t *lengthIn) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    requireNotNull(lengthIn, SETTINGS_ERROR_INVALID_PARAMETER, "lengthIn cannot be a NULL pointer");
    esp_err_t err = nvs_get_str(nvsDefaultHandle, key, NULL, lengthIn);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        throw(SETTINGS_ERROR_KEY_NOT_FOUND, "Key %s does not exist", key);
    } else if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(SETTINGS_ERROR_NVS_CLOSED, "NVS handle is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(SETTINGS_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE,
              "Parameter `length` is not large enough to contain data, this should never happen");
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_get_str() returned %s", esp_err_to_name(err));
    }
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_getString(const SettingsKey *key,
                                        char *valueIn) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    size_t requiredLength = 0;
    esp_err_t err = nvs_get_str(nvsDefaultHandle, key, NULL, &requiredLength);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        throw(SETTINGS_ERROR_KEY_NOT_FOUND, "Key %s does not exist", key);
    } else if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(SETTINGS_ERROR_NVS_CLOSED, "NVS handle is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(SETTINGS_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE,
              "Parameter `length` is not large enough to contain data, this should have been handled by %s",
              __PRETTY_FUNCTION__);
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_get_str() returned %s", esp_err_to_name(err));
    }
    size_t stringLength;
    if (valueIn == NULL) { // allocate to fit
        valueIn = alloc(requiredLength);
        stringLength = requiredLength;
    } else {
        stringLength = strlen(valueIn);
    }
    err = nvs_get_str(nvsDefaultHandle, key, valueIn, &stringLength);
    if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        // TODO: 31-Jul-2022 @basshelal: Unit Test this condition
        throw(SETTINGS_ERROR_INVALID_LENGTH,
              "String `valueIn` was not large enough to contain the result given size: %i, required size: %i",
              stringLength, requiredLength);
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_get_str() returned %s", esp_err_to_name(err));
    }
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_putUInt32(const SettingsKey *key, const uint32_t value) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    esp_err_t err = nvs_set_u32(nvsDefaultHandle, key, value);
    if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(SETTINGS_ERROR_NVS_CLOSED, "NVS handled is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(SETTINGS_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        throw(SETTINGS_ERROR_NOT_ENOUGH_SPACE, "Not enough space in NVS to save key: %s, value: %i",
              key, value);
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_set_i32() returned %s", esp_err_to_name(err));
    }
    err = nvs_commit(nvsDefaultHandle);
    if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_commit() returned %s", esp_err_to_name(err));
    }
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_getUInt32(const SettingsKey *key,
                                        uint32_t *valueIn) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    esp_err_t err = nvs_get_u32(nvsDefaultHandle, key, valueIn);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        throw(SETTINGS_ERROR_KEY_NOT_FOUND, "Key %s does not exist", key);
    } else if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(SETTINGS_ERROR_NVS_CLOSED, "NVS handle is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(SETTINGS_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE,
              "Parameter `length` is not large enough to contain data, this should never happen");
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_get_i32() returned %s", esp_err_to_name(err));
    }
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_deleteKey(const SettingsKey *key) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    esp_err_t err = nvs_erase_key(nvsDefaultHandle, key);
    if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(SETTINGS_ERROR_NVS_CLOSED, "NVS handle is closed");
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        throw(SETTINGS_ERROR_KEY_NOT_FOUND, "Key %s does not exist", key);
    } else if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_erase_key() returned %s", esp_err_to_name(err));
    }
    err = nvs_commit(nvsDefaultHandle);
    if (err != ESP_OK) {
        throw(SETTINGS_ERROR_GENERIC_FAILURE, "nvs_erase_key() returned %s", esp_err_to_name(err));
    }
    return SETTINGS_ERROR_NONE;
}

public SettingsError settings_hasKey(const SettingsKey *key, bool *hasKey) {
    requireNotNull(key, SETTINGS_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    bool result = false;
    nvs_iterator_t it = nvs_entry_find(NVS_PARTITION, NVS_NAMESPACE, NVS_TYPE_ANY);
    nvs_entry_info_t info;
    while (it != NULL) {
        nvs_entry_info(it, &info);
        if (info.key == key) {
            result = true;
            break;
        }
        it = nvs_entry_next(it);
    }
    nvs_release_iterator(it);
    return result;
}