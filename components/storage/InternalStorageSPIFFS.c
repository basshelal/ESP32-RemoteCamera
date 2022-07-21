#include "InternalStorage.h"
#include "InternalStorageCommon.h"
#include "InternalStorageSPIFFS.h"
#include "Logger.h"
#include <esp_err.h>
#include <esp_spiffs.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define SPIFFS_PATH "/spiffs"

public StorageError spiffs_init() {
    VERBOSE("Initializing SPIFFS");
    const esp_vfs_spiffs_conf_t conf = {
            .base_path = SPIFFS_PATH,
            .partition_label = NULL,
            .max_files = 5,
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
    VERBOSE("Successfully initialized SPIFFS");
    return STORAGE_ERROR_NONE;
}

public StorageError spiffs_destroy() {
    VERBOSE("Destroying SPIFFS");
    esp_err_t err = esp_vfs_spiffs_unregister(NULL);
    if (err == ESP_ERR_INVALID_STATE) {
        WARN("SPIFFS already unregistered");
    }
    VERBOSE("Successfully destroyed SPIFFS");
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_readFile(const char *filePath,
                                                 void *bufferIn in_parameter,
                                                 const uint bufferLength,
                                                 uint *bytesRead in_parameter) {
    if (filePath == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "filePath cannot be a NULL pointer");
    }
    if (bytesRead == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "bytesRead cannot be a NULL pointer");
    }
    if (bufferIn == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "bufferIn cannot be a NULL pointer");
    }

    char path[128];
    int err = sprintf(path, "%s/%s", SPIFFS_PATH, filePath);
    if (err < 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() returned %i: %s", err, strerror(err));
    }

    const FILE *file = fopen(path, "r");
    if (file == NULL) {
        err = errno;
        if (err == ENOENT) {
            throw(STORAGE_ERROR_FILE_NOT_FOUND,
                  "fopen() returned ENOENT, file in path: %s was likely not found", path);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "fopen() returned error %i: %s", err, strerror(err));
        }
    }

    *bytesRead = fread(bufferIn, sizeof(char), bufferLength, file);
    if (ferror(file)) {
        err = errno;
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fread() returned error %i: %s", err, strerror(err));
    }

    err = fclose(file);
    if (err != 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fclose() returned error %i: %s", err, strerror(err));
    }

    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_writeFile(const char *filePath,
                                                  const void *buffer in_parameter,
                                                  const uint bufferLength,
                                                  uint *bytesWritten in_parameter) {

    if (filePath == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "filePath cannot be a NULL pointer");
    }
    if (bytesWritten == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "bytesWritten cannot be a NULL pointer");
    }
    if (buffer == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "buffer cannot be a NULL pointer");
    }

    char path[128];
    int err = sprintf(path, "%s/%s", SPIFFS_PATH, filePath);
    if (err < 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() returned %i: %s", err, strerror(err));
    }

    const FILE *file = fopen(path, "w");
    if (file == NULL) {
        err = errno;
        if (err == ENOENT) {
            throw(STORAGE_ERROR_FILE_NOT_FOUND,
                  "fopen() returned ENOENT, file in path: %s was likely not found", path);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "fopen() returned error %i: %s", err, strerror(err));
        }
    }

    *bytesWritten = fwrite(buffer, sizeof(char), bufferLength, file);
    if (ferror(file)) {
        err = errno;
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fwrite() returned error %i: %s", err, strerror(err));
    }

    err = fclose(file);
    if (err != 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fclose() returned error %i: %s", err, strerror(err));
    }

    return STORAGE_ERROR_NONE;
}

public bool internalStorage_queryFileExists(const char *filePath) {
    if (filePath == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "filePath cannot be a NULL pointer");
    }
    char path[128];
    int err = sprintf(path, "%s/%s", SPIFFS_PATH, filePath);
    if (err < 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() returned %i: %s", err, strerror(err));
    }
    struct stat statStruct;
    err = stat(path, &statStruct);
    return err == 0;
}

public StorageError internalStorage_queryFileSize(const char *filePath,
                                                      uint *fileSizeBytesIn in_parameter) {
    if (filePath == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "filePath cannot be a NULL pointer");
    }
    if (fileSizeBytesIn == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "fileSizeBytesIn cannot be a NULL pointer");
    }
    char path[128];
    int err = sprintf(path, "%s/%s", SPIFFS_PATH, filePath);
    if (err < 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() returned %i: %s", err, strerror(err));
    }

    struct stat statStruct;
    err = stat(path, &statStruct);
    if (err != 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "stat() returned %i: %s", err, strerror(err));
    }
    *fileSizeBytesIn = statStruct.st_size;
    return STORAGE_ERROR_NONE;
}

public StorageError internalStorage_deleteFile(const char *filePath) {
    if (filePath == NULL) {
        throw(STORAGE_ERROR_INVALID_PARAMETER, "filePath cannot be a NULL pointer");
    }
    char path[128];
    int err = sprintf(path, "%s/%s", SPIFFS_PATH, filePath);
    if (err < 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() returned %i: %s", err, strerror(err));
    }
    err = remove(path);
    if (err == ENOENT) {
        throw(STORAGE_ERROR_FILE_NOT_FOUND, "File with path: %s does not exist", path);
    } else if (err != 0) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "remove() for file path: %s returned error %i: %s",
              path, err, strerror(err));
    }
    return STORAGE_ERROR_NONE;
}