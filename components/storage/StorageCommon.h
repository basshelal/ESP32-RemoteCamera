#ifndef ESP32_REMOTECAMERA_STORAGECOMMON_H
#define ESP32_REMOTECAMERA_STORAGECOMMON_H

#include "ff.h"
#include "Constants.h"
#include "StorageError.h"
#include "StorageInfo.h"
#include "FileMode.h"

#define requireParamNotNull(element) \
requireNotNull(element, STORAGE_ERROR_INVALID_PARAMETER, #element" cannot be NULL")

// TODO: 16-Sep-2022 @basshelal: Should we use this? This adds a performance hit since we may have to check every time
#define requirePathLengthUnderLimit(path, limit) \
size_t _length = strlen(path); \
require(_length <= limit, STORAGE_ERROR_INVALID_LENGTH, \
        "path length cannot exceed limit of %u, was %u", limit, _length)

#define getPrefixedPath(buffer, prefix, path) \
do {if (sprintf(buffer, "%s/%s", prefix, path) < 0) { \
throw(STORAGE_ERROR_GENERIC_FAILURE, "sprintf() failed for %s", path); \
}}while (0)

/*============================= Directories =================================*/

extern StorageError storage_queryDirExists(const char *dirPath, bool *dirExists);

extern StorageError storage_queryDirInfo(const char *dirPath, DirInfo *dirInfo);

extern StorageError storage_createDir(const char *dirPath);

extern StorageError storage_readDir(const char *dirPath, char **dirEntries, size_t *entryCount);

extern StorageError storage_moveDir(const char *dirPath, const char *newDirPath);

extern StorageError storage_deleteDir(const char *dirPath);

/*============================= Files =======================================*/

extern StorageError storage_queryFileExists(const char *filePath, bool *fileExists);

extern StorageError storage_queryFileInfo(const char *filePath, FileInfo *fileInfo);

extern StorageError storage_createFile(const char *filePath);

extern StorageError storage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode);

extern StorageError storage_closeFile(const FILE *fileIn);

extern StorageError storage_readFile(const FILE *file, const size_t startPosition,
                                     void *bufferIn, const uint bufferLength, uint *bytesRead);

extern StorageError storage_writeFile(const FILE *file, const size_t startPosition,
                                      const void *buffer, const uint bufferLength, uint *bytesWritten);

extern StorageError storage_moveFile(const char *filePath, const char *newFilePath);

extern StorageError storage_deleteFile(const char *filePath);

inline const char *fresultToString(const FRESULT fresult) {
#if CONFIG_USE_ERROR_TO_STRING
    switch (fresult) {
        case FR_OK:
            return "FR_OK";
        case FR_DISK_ERR:
            return "FR_DISK_ERR";
        case FR_INT_ERR:
            return "FR_INT_ERR";
        case FR_NOT_READY:
            return "FR_NOT_READY";
        case FR_NO_FILE:
            return "FR_NO_FILE";
        case FR_NO_PATH:
            return "FR_NO_PATH";
        case FR_INVALID_NAME:
            return "FR_INVALID_NAME";
        case FR_DENIED:
            return "FR_DENIED";
        case FR_EXIST:
            return "FR_EXIST";
        case FR_INVALID_OBJECT:
            return "FR_INVALID_OBJECT";
        case FR_WRITE_PROTECTED:
            return "FR_WRITE_PROTECTED";
        case FR_INVALID_DRIVE:
            return "FR_INVALID_DRIVE";
        case FR_NOT_ENABLED:
            return "FR_NOT_ENABLED";
        case FR_NO_FILESYSTEM:
            return "FR_NO_FILESYSTEM";
        case FR_MKFS_ABORTED:
            return "FR_MKFS_ABORTED";
        case FR_TIMEOUT:
            return "FR_TIMEOUT";
        case FR_LOCKED:
            return "FR_LOCKED";
        case FR_NOT_ENOUGH_CORE:
            return "FR_NOT_ENOUGH_CORE";
        case FR_TOO_MANY_OPEN_FILES:
            return "FR_TOO_MANY_OPEN_FILES";
        case FR_INVALID_PARAMETER:
            return "FR_INVALID_PARAMETER";
        default:
            return "Unknown Error";
    }
#else
    return "";
#endif // CONFIG_USE_ERROR_TO_STRING
}

#endif //ESP32_REMOTECAMERA_STORAGECOMMON_H
