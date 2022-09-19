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

public Error internalStorage_init() {
    if (this.isInitialized) {
        WARN("Internal storage has already initialized!");
        return ERROR_NONE;
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
        throw(ERROR_ILLEGAL_ARGUMENT, "Partition for SPIFFS was not found");
    } else if (err == ESP_ERR_INVALID_STATE) {
        throw(ERROR_ILLEGAL_STATE, "Partition already mounted or encrypted");
    } else if (err != ESP_OK) {
        throwESPError(esp_vfs_spiffs_register(), err);
    }
    VERBOSE("Successfully initialized internal storage");
    this.isInitialized = true;
    return ERROR_NONE;
}

public Error internalStorage_destroy() {
    if (!this.isInitialized) {
        throw(ERROR_NOT_INITIALIZED,
              "could not destroy internal storage, internal storage was not initialized");
    }
    VERBOSE("Destroying internal storage");
    esp_err_t err = esp_vfs_spiffs_unregister(SPIFFS_PARTITION);
    if (err == ESP_ERR_INVALID_STATE) {
        WARN("SPIFFS already unregistered");
    }
    VERBOSE("Successfully destroyed internal storage");
    this.isInitialized = false;
    return ERROR_NONE;
}

public Error internalStorage_queryFileExists(const char *filePath, bool *fileExists) {
    requireArgNotNull(filePath);
    getPath(path, filePath);

    return storage_queryFileExists(path, fileExists);
}

public Error internalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo) {
    requireArgNotNull(filePath);
    requireArgNotNull(fileInfo);
    getPath(path, filePath);

    return storage_queryFileInfo(path, fileInfo);
}

public Error internalStorage_createFile(const char *filePath) {
    requireArgNotNull(filePath);
    getPath(path, filePath);

    return storage_createFile(path);
}

public Error internalStorage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode) {
    requireArgNotNull(filePath);
    requireArgNotNull(fileIn);
    getPath(path, filePath);

    return storage_openFile(path, fileIn, fileMode);
}

public Error internalStorage_closeFile(const FILE *file) {
    requireArgNotNull(file);

    return storage_closeFile(file);
}

public Error internalStorage_readFile(const FILE *file, const size_t startPosition,
                                      void *bufferIn, const uint bufferLength, uint *bytesRead) {
    requireArgNotNull(file);
    requireArgNotNull(bufferIn);
    requireArgNotNull(bytesRead);

    return storage_readFile(file, startPosition, bufferIn, bufferLength, bytesRead);
}

public Error internalStorage_writeFile(const FILE *file, const size_t startPosition,
                                       const void *buffer, const uint bufferLength, uint *bytesWritten) {
    requireArgNotNull(file);
    requireArgNotNull(buffer);
    requireArgNotNull(bytesWritten);

    return storage_writeFile(file, startPosition, buffer, bufferLength, bytesWritten);
}

public Error internalStorage_moveFile(const char *filePath, const char *newFilePath) {
    requireArgNotNull(filePath);
    requireArgNotNull(newFilePath);

    getPath(oldPath, filePath);
    getPath(newPath, newFilePath);

    return storage_moveFile(oldPath, newPath);
}

public Error internalStorage_deleteFile(const char *filePath) {
    requireArgNotNull(filePath);
    getPath(path, filePath);

    return storage_deleteFile(path);
}