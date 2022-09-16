#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGE_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGE_H

#include "StorageError.h"
#include "StorageInfo.h"
#include "FileMode.h"
#include "Utils.h"
#include <stdio.h>
#include <stdbool.h>

extern StorageError internalStorage_init();

/** Destroys and de-initializes all internal structures,
 * call only when you know a restart or shutdown will definitely happen and internal storage will no longer be used */
extern StorageError internalStorage_destroy();

extern StorageError internalStorage_queryFileExists(const char *filePath, bool *fileExists);

extern StorageError internalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo);

extern StorageError internalStorage_createFile(const char *filePath);

extern StorageError internalStorage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode);

extern StorageError internalStorage_closeFile(const FILE *fileIn);

extern StorageError internalStorage_readFile(const FILE *file, const size_t startPosition,
                                             void *bufferIn, const uint bufferLength, uint *bytesRead);

extern StorageError internalStorage_writeFile(const FILE *file, const size_t startPosition,
                                              const void *buffer, const uint bufferLength, uint *bytesWritten);

extern StorageError internalStorage_moveFile(const char *filePath, const char *newFilePath);

extern StorageError internalStorage_deleteFile(const char *filePath);

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGE_H
