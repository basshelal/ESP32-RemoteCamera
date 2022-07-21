#include "InternalStorage.h"
#include "InternalStorageCommon.h"
#include "InternalStorageNVS.h"
#include "Logger.h"
#include <esp_err.h>
#include <nvs_flash.h>

#define NVS_PARTITION NVS_DEFAULT_PART_NAME
#define NVS_NAMESPACE "default"

private nvs_handle_t nvsDefaultHandle;

public StorageError nvs_init() {
    VERBOSE("Initializing NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NOT_FOUND) {
        throw(STORAGE_ERROR_PARTITION_NOT_FOUND, "No partition with label \"%s\", cannot proceed!", NVS_PARTITION);
    }
    if (err == ESP_ERR_NVS_NO_FREE_PAGES //  NVS storage contains no empty pages (partition was truncated)
        || err == ESP_ERR_NVS_NEW_VERSION_FOUND) { // NVS partition contains data in new format
        VERBOSE("nvs_flash_init() failed, erasing and retrying");
        err = nvs_flash_erase();
        if (err == ESP_ERR_NOT_FOUND) {
            throw(STORAGE_ERROR_PARTITION_NOT_FOUND, "No partition with label \"%s\", cannot proceed!",
                  NVS_PARTITION);
        } else {
            err = nvs_flash_init();
            if (err != ESP_OK) {
                throw(STORAGE_ERROR_GENERIC_FAILURE, "NVS init failed after retry, error: %s", esp_err_to_name(err));
            }
            VERBOSE("nvs_flash_init() retry successful");
        }
    }
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvsDefaultHandle);
    if (err == ESP_ERR_NVS_PART_NOT_FOUND) {
        throw(STORAGE_ERROR_PARTITION_NOT_FOUND, "No partition with label \"%s\", cannot proceed!", NVS_PARTITION);
    } else if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "NVS driver was not initialized");
    } else if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_open() returned error: %s", esp_err_to_name(err));
    }
    VERBOSE("Successfully initialized NVS");
    return STORAGE_ERROR_NONE;
}

public StorageError nvs_destroy() {
    VERBOSE("Destroying NVS");
    nvs_close(nvsDefaultHandle);
    esp_err_t err = nvs_flash_deinit();
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        WARN("nvs_flash_deinit() returned: %s, NVS was not initialized", esp_err_to_name(err));
    }
    VERBOSE("Successfully destroyed NVS");
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_putString(const InternalStorageKey *key nonnull, const char *value nonnull) {
    requireNotNull(key, STORAGE_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    requireNotNull(value, STORAGE_ERROR_INVALID_PARAMETER, "value cannot be a NULL pointer");
    esp_err_t err = nvs_set_str(nvsDefaultHandle, key, value);
    if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(STORAGE_ERROR_NVS_CLOSED, "NVS handled is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(STORAGE_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE) {
        throw(STORAGE_ERROR_NOT_ENOUGH_SPACE, "Not enough space in NVS to save key: %s, value: %s",
              key, value);
    } else if (err == ESP_ERR_NVS_VALUE_TOO_LONG) {
        throw(STORAGE_ERROR_INVALID_VALUE,
              "Value too long, max size is 4000 bytes at best (including terminator), value: %s", value);
    } else if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_set_str() returned %s", esp_err_to_name(err));
    }
    err = nvs_commit(nvsDefaultHandle);
    if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_commit() returned %s", esp_err_to_name(err));
    }
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_getString(const InternalStorageKey *key nonnull,
                                              char *valueIn in_parameter nonnull) {
    requireNotNull(key, STORAGE_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    requireNotNull(valueIn, STORAGE_ERROR_INVALID_PARAMETER, "valueIn cannot be a NULL pointer");
    size_t length = 0;
    esp_err_t err = nvs_get_str(nvsDefaultHandle, key, NULL, &length);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        throw(STORAGE_ERROR_KEY_NOT_FOUND, "Key %s does not exist", key);
    } else if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(STORAGE_ERROR_NVS_CLOSED, "NVS handle is closed");
    } else if (err == ESP_ERR_NVS_INVALID_NAME) {
        throw(STORAGE_ERROR_INVALID_KEY,
              "Key: %s doesn't satisfy key name constraints, should be %i chars long (including terminator)",
              key, NVS_KEY_NAME_MAX_SIZE);
    } else if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        throw(STORAGE_ERROR_GENERIC_FAILURE,
              "Parameter `length` is not large enough to contain data, this should have been handled by %s",
              __PRETTY_FUNCTION__);
    } else if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_get_str() returned %s", esp_err_to_name(err));
    }
    err = nvs_get_str(nvsDefaultHandle, key, valueIn, &length);
    if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        throw(STORAGE_ERROR_GENERIC_FAILURE,
              "Parameter `length` is not large enough to contain data, this should have been handled by %s",
              __PRETTY_FUNCTION__);
    } else if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_get_str() returned %s", esp_err_to_name(err));
    }
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_deleteKey(const InternalStorageKey *key nonnull) {
    requireNotNull(key, STORAGE_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
    esp_err_t err = nvs_erase_key(nvsDefaultHandle, key);
    if (err == ESP_ERR_NVS_INVALID_HANDLE) {
        throw(STORAGE_ERROR_NVS_CLOSED, "NVS handle is closed");
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        throw(STORAGE_ERROR_KEY_NOT_FOUND, "Key %s does not exist", key);
    } else if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_erase_key() returned %s", esp_err_to_name(err));
    }
    err = nvs_commit(nvsDefaultHandle);
    if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "nvs_erase_key() returned %s", esp_err_to_name(err));
    }
    return STORAGE_ERROR_NONE;
}

public bool internalStorage_hasKey(const InternalStorageKey *key nonnull) {
    requireNotNull(key, STORAGE_ERROR_INVALID_PARAMETER, "key cannot be a NULL pointer");
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