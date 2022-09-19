#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGE_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGE_H

#include "Error.h"
#include "StorageInfo.h"
#include "FileMode.h"
#include "Utils.h"
#include <stdio.h>
#include <stdbool.h>

extern Error internalStorage_init();

/** Destroys and de-initializes all internal structures,
 * call only when you know a restart or shutdown will definitely happen and internal storage will no longer be used */
extern Error internalStorage_destroy();

extern Error internalStorage_queryFileExists(const char *filePath, bool *fileExists);

extern Error internalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo);

extern Error internalStorage_createFile(const char *filePath);

extern Error internalStorage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode);

extern Error internalStorage_closeFile(const FILE *fileIn);

extern Error internalStorage_readFile(const FILE *file, const size_t startPosition,
                                             void *bufferIn, const uint bufferLength, uint *bytesRead);

extern Error internalStorage_writeFile(const FILE *file, const size_t startPosition,
                                              const void *buffer, const uint bufferLength, uint *bytesWritten);

extern Error internalStorage_moveFile(const char *filePath, const char *newFilePath);

extern Error internalStorage_deleteFile(const char *filePath);

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGE_H
