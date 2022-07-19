#include "Storage.h"
#include "StorageCommon.h"
#include "InternalStorage.h"
#include "InternalStorageCommon.h"
#include "InternalStorageSPIFFS.h"
#include "Logger.h"
#include <esp_err.h>
#include <esp_spiffs.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define SPIFFS_PATH "/spiffs"

public StorageError spiffs_init() {
    const esp_vfs_spiffs_conf_t conf = {
            .base_path = SPIFFS_PATH,
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = false
    };
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    // TODO: 19-Jul-2022 @basshelal: Error check!

    TODO("No error checking yet!");
}

public StorageError spiffs_destroy() {
    esp_vfs_spiffs_unregister(NULL);
    // TODO: 19-Jul-2022 @basshelal: Error check!
    TODO("No error checking yet!");
}

undeclared bool internalStorage_checkFileExists(const char *filePath) {
    int err = access(filePath, F_OK);
    return err == 0;
}

// TODO: 19-Jul-2022 @basshelal: Use more specific functions!
//  readFile() writeFile() appendFile()
// Get, if not exists fileIn is NULL and error is returned
undeclared StorageError internalStorage_openFile(const char *filePath, FILE *fileIn,
                                                 const FileAccessMode accessMode, bool createIfNotExists) {
    const bool exists = internalStorage_checkFileExists(filePath);
    if (!exists && !createIfNotExists) {
        throw(STORAGE_ERROR_FILE_NOT_FOUND, "File: %s does not exist", filePath);
    }
    char path[128];
    int err = sprintf(path, "%s/%s", SPIFFS_PATH, filePath);
    if (err < 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() returned %i: %s", err, strerror(err));
    }
    const char *accessModeString = fileAccessModeToString(accessMode);
    if (accessModeString == NULL) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "Invalid accessMode: %i", accessMode);
    }
    fileIn = fopen(path, accessModeString);
    if (fileIn == NULL) {
        err = errno;
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fopen() returned %i: %s", err, strerror(err));
    }
    return STORAGE_ERROR_NONE;
}

// Delete if exists, error if not
undeclared StorageError internalStorage_deleteFile(const char *filePath) {
    // TODO: 19-Jul-2022 @basshelal: Implement properly
    remove(filePath);
    return STORAGE_ERROR_NONE;
}