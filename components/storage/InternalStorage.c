#include "InternalStorage.h"
#include "InternalStorageCommon.h"
#include "InternalStorageNVS.h"
#include "InternalStorageSPIFFS.h"
#include "Logger.h"

public StorageError internalStorage_init() {
    VERBOSE("Initializing internal storage");
    StorageError nvsError = nvs_init();
    StorageError spiffsError = spiffs_init();
    if (nvsError != STORAGE_ERROR_NONE || spiffsError != STORAGE_ERROR_NONE) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "Could not initialize internal storage");
    }
    VERBOSE("Successfully initialized internal storage");
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