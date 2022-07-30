#include "InternalStorage.h"
#include "InternalStorageCommon.h"
#include "InternalStorageNVS.h"
#include "InternalStorageSPIFFS.h"
#include "Logger.h"

private bool hasInitialized = false;
public StorageError internalStorage_init() {
    if (!hasInitialized) {
        VERBOSE("Initializing internal storage");
        StorageError nvsError = nvs_init();
        StorageError spiffsError = spiffs_init();
        if (nvsError != STORAGE_ERROR_NONE || spiffsError != STORAGE_ERROR_NONE) {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "Could not initialize internal storage");
        }
        VERBOSE("Successfully initialized internal storage");
        hasInitialized = true;
    } else {
        WARN("Internal storage has already initialized!");
    }
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_destroy() {
    VERBOSE("Destroying internal storage");
    StorageError nvsError = nvs_destroy();
    StorageError spiffsError = spiffs_destroy();
    if (nvsError != STORAGE_ERROR_NONE || spiffsError != STORAGE_ERROR_NONE) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "Could not properly destroy internal storage");
    }
    VERBOSE("Successfully destroyed internal storage");
    return STORAGE_ERROR_NONE;
}