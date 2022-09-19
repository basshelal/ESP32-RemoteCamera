#ifndef ESP32_REMOTECAMERA_EXTERNALSTORAGE_H
#define ESP32_REMOTECAMERA_EXTERNALSTORAGE_H

#include "Error.h"
#include "StorageInfo.h"
#include "FileMode.h"
#include "Constants.h"
#include "Utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <ff.h>

/** Max length a file or dir name can be */
#define EXTERNAL_STORAGE_MAX_FILE_LENGTH FF_MAX_LFN
/** Max length a full path can be including dots, slashes, extensions and the mount prefix */
#define EXTERNAL_STORAGE_MAX_PATH_LENGTH 260

typedef struct {
    bool startAutoMountTask;
} ExternalStorageOptions;

#define EXTERNAL_STORAGE_DEFAULT_OPTIONS {.startAutoMountTask=true,}

/** Call before any external storage operations including to check if we have any external storage */
extern Error externalStorage_init(ExternalStorageOptions *externalStorageOptions);

extern Error externalStorage_destroy();

extern Error externalStorage_unmountSDCard();

extern bool externalStorage_hasSDCard();

extern Error externalStorage_getStorageInfo(StorageInfo *storageInfo);

/*============================= Directories =================================*/

extern Error externalStorage_queryDirExists(const char *dirPath, bool *dirExists);

extern Error externalStorage_queryDirInfo(const char *dirPath, DirInfo *dirInfo);

// Create if not exists, else return error
extern Error externalStorage_createDir(const char *dirPath);

// if dirEntries is NULL writes to entryCount the number of entries in the dir
extern Error externalStorage_readDir(const char *dirPath, char **dirEntries, size_t *entryCount);

extern Error externalStorage_moveDir(const char *dirPath, const char *newDirPath);

extern Error externalStorage_deleteDir(const char *dirPath);

/*============================= Files =======================================*/

extern Error externalStorage_queryFileExists(const char *filePath,
                                                    bool *fileExists);

extern Error externalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo);

extern Error externalStorage_createFile(const char *filePath);

extern Error externalStorage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode);

extern Error externalStorage_closeFile(const FILE *fileIn);

extern Error externalStorage_readFile(const FILE *file, const size_t startPosition,
                                             void *bufferIn, const uint bufferLength, uint *bytesRead);

extern Error externalStorage_writeFile(const FILE *file, const size_t startPosition,
                                              const void *buffer, const uint bufferLength, uint *bytesWritten);

// Rename is a move operation like Unix
extern Error externalStorage_moveFile(const char *filePath, const char *newFilePath);

extern Error externalStorage_deleteFile(const char *filePath);

#endif //ESP32_REMOTECAMERA_EXTERNALSTORAGE_H
