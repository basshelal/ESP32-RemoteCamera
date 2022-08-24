#ifndef ESP32_REMOTECAMERA_EXTERNALSTORAGE_H
#define ESP32_REMOTECAMERA_EXTERNALSTORAGE_H

#include "Utils.h"
#include "StorageError.h"
#include "FileMode.h"
#include "Info.h"
#include <stdio.h>
#include <stdbool.h>

/** Call before any external storage operations including to check if we have any external storage */
extern StorageError externalStorage_init();

/** Destroys and de-initializes all external structures */
extern StorageError externalStorage_destroy();

extern StorageError externalStorage_unmountSDCard();

extern bool externalStorage_hasSDCard();

extern StorageError externalStorage_getSDCardInfo(SDCardInfo *sdCardInfo);

extern StorageError externalStorage_getStorageInfo(StorageInfo *storageInfo);

/*============================= Directories =================================*/

extern StorageError externalStorage_queryDirExists(const char *dirPath, bool *dirExists);

extern StorageError externalStorage_createDir(const char *dirPath);

extern StorageError externalStorage_readDir(const char *dirPath, char **dirEntries, size_t *entryCount);

extern StorageError externalStorage_moveDir(const char *dirPath, const char *newDirPath);

extern StorageError externalStorage_deleteDir(const char *dirPath);

extern StorageError externalStorage_queryDirInfo(const char *dirPath, DirInfo *dirInfo);

/*============================= Files =======================================*/

extern StorageError externalStorage_queryFileExists(const char *filePath,
                                                    bool *fileExists);

extern StorageError externalStorage_createFile(const char *filePath);

extern StorageError externalStorage_openFile(const char *filePath,
                                             FILE **fileIn,
                                             const FileMode fileMode);

extern StorageError externalStorage_closeFile(const FILE *fileIn);

extern StorageError externalStorage_readFile(const FILE *file,
                                             size_t startPosition,
                                             void *bufferIn,
                                             const uint bufferLength,
                                             uint *bytesRead);

extern StorageError externalStorage_writeFile(const char *filePath,
                                              size_t startPosition,
                                              const void *buffer,
                                              const uint bufferLength,
                                              uint *bytesWritten);

// Rename is a move operation like Unix
extern StorageError externalStorage_moveFile(const char *filePath, const char *newFilePath);

extern StorageError externalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo);

extern StorageError externalStorage_deleteFile(const char *filePath);

#endif //ESP32_REMOTECAMERA_EXTERNALSTORAGE_H
