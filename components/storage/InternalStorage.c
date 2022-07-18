#include "InternalStorage.h"
#include "Logger.h"
#include <esp_err.h>
#include <nvs_flash.h>

typedef void InternalStorageKey;

#define TAG __FILE_NAME__
#define LINE __LINE__
#define FUNC __PRETTY_FUNCTION__
#define LOG_TEMPLATE(s, ...) TAG, "%s %d"s, FUNC, LINE, ##__VA_ARGS__
#define undeclared static

private esp_err_t err;
private nvs_handle_t nvsHandle;

private inline esp_err_t nvsInit() {
    logI(LOG_TEMPLATE("Initializing NVS"));
    err = nvs_flash_init();
    if (err == ESP_ERR_NOT_FOUND) { // no partition with label "nvs" is found in the partition table
        logE(LOG_TEMPLATE("No partition with label NVS, cannot proceed!"));
        return ESP_FAIL;
    }
    if (err == ESP_ERR_NVS_NO_FREE_PAGES //  NVS storage contains no empty pages (partition was truncated)
        || err == ESP_ERR_NVS_NEW_VERSION_FOUND) { // NVS partition contains data in new format
        err = nvs_flash_erase();
        if (err == ESP_ERR_NOT_FOUND) { // no partition with label "nvs" is found in the partition table
            logE(LOG_TEMPLATE("No partition with label NVS, cannot proceed!"));
            return ESP_FAIL;
        } else {
            err = nvs_flash_init();
            if (err != ESP_OK) {
                logE(LOG_TEMPLATE("NVS init failed after retry, error: %s", esp_err_to_name(err)));
                return ESP_FAIL;
            }
        }
    }
    return ESP_OK;
}

public StorageError internalStorage_init() {
    logI(LOG_TEMPLATE("Initializing Internal Storage"));
    err = nvsInit();
    if (nvsInit() == ESP_FAIL) {
        return STORAGE_ERROR_NVS_INIT_FAIL;
    }
    err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    // TODO: 18-Jul-2022 @basshelal: Error check

    // TODO: 18-Jul-2022 @basshelal: Init SPIFFS for filesystem
    return STORAGE_ERROR_NONE;
}

// Useful when you know a restart will happen, only after successful init
undeclared StorageError internalStorage_destroy() {
    nvs_close(nvsHandle);
    nvs_flash_deinit();
    // TODO: 18-Jul-2022 @basshelal: Error check

    return STORAGE_ERROR_NONE;
}

// Put if not exists or set if exists, check exists by calling get
undeclared StorageError internalStorage_putKeyValue(const InternalStorageKey *key, const char *value) {
    nvs_set_str(nvsHandle, key, value);
    nvs_commit(nvsHandle);
    // TODO: 18-Jul-2022 @basshelal: Error checks on both!
    return STORAGE_ERROR_NONE;
}

// Get, if not exists valueIn is NULL and an error is returned
undeclared StorageError internalStorage_getKeyValue(const InternalStorageKey *key, char *valueIn) {
    size_t length = 0;
    nvs_get_str(nvsHandle, key, valueIn, &length);
    // TODO: 18-Jul-2022 @basshelal: Error check
    return STORAGE_ERROR_NONE;
}

// Delete if exists, error if not
undeclared StorageError internalStorage_deleteKeyValue(const InternalStorageKey *key) {
    nvs_erase_key(nvsHandle, key);
    nvs_commit(nvsHandle);
    // TODO: 18-Jul-2022 @basshelal: Error check
    return STORAGE_ERROR_NONE;
}

// Put if not exists or set if exists, check exists by calling get
undeclared StorageError internalStorage_putFile(const char *filePath, const FILE *file) {
    return STORAGE_ERROR_NONE;
}

// Get, if not exists fileIn is NULL and error is returned
undeclared StorageError internalStorage_getFile(const char *filePath, FILE *fileIn) {
    return STORAGE_ERROR_NONE;
}

// Delete if exists, error if not
undeclared StorageError internalStorage_deleteFile(const char *filePath) {
    return STORAGE_ERROR_NONE;
}