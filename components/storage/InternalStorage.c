#include "InternalStorage.h"
#include "StorageCommon.h"
#include "Logger.h"
#include <esp_err.h>
#include <esp_spiffs.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define SPIFFS_PATH "/spiffs"
#define SPIFFS_PARTITION "storage"
#define SPIFFS_MAX_PATH_LENGTH CONFIG_SPIFFS_OBJ_NAME_LEN

#define getPath(name, path) \
char name[SPIFFS_MAX_PATH_LENGTH];   \
getPrefixedPath(name, SPIFFS_PATH, path)

private struct {
    bool isInitialized;
} this;

public StorageError internalStorage_init() {
    if (this.isInitialized) {
        WARN("Internal storage has already initialized!");
        return STORAGE_ERROR_NONE;
    }
    VERBOSE("Initializing internal storage");
    const esp_vfs_spiffs_conf_t conf = {
            .base_path = SPIFFS_PATH,
            .partition_label = SPIFFS_PARTITION,
            .max_files = 8,
            .format_if_mount_failed = false
    };
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err == ESP_ERR_NOT_FOUND) {
        throw(STORAGE_ERROR_PARTITION_NOT_FOUND, "Partition for SPIFFS was not found");
    } else if (err == ESP_ERR_INVALID_STATE) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "Partition already mounted or encrypted");
    } else if (err != ESP_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "esp_vfs_spiffs_register() returned error: %s", esp_err_to_name(err));
    }
    VERBOSE("Successfully initialized internal storage");
    this.isInitialized = true;
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_destroy() {
    if (!this.isInitialized) {
        throw(STORAGE_ERROR_GENERIC_FAILURE,
              "could not destroy internal storage, internal storage was not initialized");
    }
    VERBOSE("Destroying internal storage");
    esp_err_t err = esp_vfs_spiffs_unregister(SPIFFS_PARTITION);
    if (err == ESP_ERR_INVALID_STATE) {
        WARN("SPIFFS already unregistered");
    }
    VERBOSE("Successfully destroyed internal storage");
    this.isInitialized = false;
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_queryFileExists(const char *filePath, bool *fileExists) {
    requireParamNotNull(filePath);
    getPath(path, filePath);

    return storage_queryFileExists(path, fileExists);
}

public StorageError internalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo) {
    requireParamNotNull(filePath);
    requireParamNotNull(fileInfo);
    getPath(path, filePath);

    return storage_queryFileInfo(path, fileInfo);
}

public StorageError internalStorage_createFile(const char *filePath) {
    requireParamNotNull(filePath);
    getPath(path, filePath);

    return storage_createFile(path);
}

public StorageError internalStorage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode) {
    requireParamNotNull(filePath);
    requireParamNotNull(fileIn);
    getPath(path, filePath);

    return storage_openFile(path, fileIn, fileMode);
}

public StorageError internalStorage_closeFile(const FILE *file) {
    requireParamNotNull(file);

    return storage_closeFile(file);
}

public StorageError internalStorage_readFile(const FILE *file, const size_t startPosition,
                                             void *bufferIn, const uint bufferLength, uint *bytesRead) {
    requireParamNotNull(file);
    requireParamNotNull(bufferIn);
    requireParamNotNull(bytesRead);

    return storage_readFile(file, startPosition, bufferIn, bufferLength, bytesRead);
}

public StorageError internalStorage_writeFile(const FILE *file, const size_t startPosition,
                                              const void *buffer, const uint bufferLength, uint *bytesWritten) {
    requireParamNotNull(file);
    requireParamNotNull(buffer);
    requireParamNotNull(bytesWritten);

    return storage_writeFile(file, startPosition, buffer, bufferLength, bytesWritten);
}

public StorageError internalStorage_moveFile(const char *filePath, const char *newFilePath) {
    requireParamNotNull(filePath);
    requireParamNotNull(newFilePath);

    getPath(oldPath, filePath);
    getPath(newPath, newFilePath);

    return storage_moveFile(oldPath, newPath);
}

public StorageError internalStorage_deleteFile(const char *filePath) {
    requireParamNotNull(filePath);
    getPath(path, filePath);

    return storage_deleteFile(path);
}