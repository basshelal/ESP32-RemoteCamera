#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGE_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGE_H

#include "Utils.h"
#include "StorageError.h"
#include <stdio.h>
#include <stdbool.h>

typedef char InternalStorageKey;

#define INTERNAL_STORAGE_KEY_WIFI_SSID "wifissid"
#define INTERNAL_STORAGE_KEY_WIFI_PASSWORD "wifipassword"
#define INTERNAL_STORAGE_KEY_WIFI_IP_ADDRESS "wifiipaddress"

#define INTERNAL_STORAGE_POSITION_CONTINUE UINT32_MAX

extern StorageError internalStorage_init();

/** Destroys and de-initializes all internal structures,
 * call only when you know a restart or shutdown will definitely happen and internal storage will no longer be used */
extern StorageError internalStorage_destroy();

/** Puts the value at key, updating it if it exists and creating it if it doesn't,
 * you can check a key's existence using \c internalStorage_hasKey */
extern StorageError internalStorage_putString(const InternalStorageKey *key, const char *value);

extern StorageError internalStorage_getString(const InternalStorageKey *key nonnull,
                                              char *valueIn in_parameter);

extern StorageError internalStorage_getStringLength(const InternalStorageKey *key nonnull,
                                                    size_t *lengthIn nonnull in_parameter);

extern StorageError internalStorage_putUInt32(const InternalStorageKey *key, const uint32_t value);

extern StorageError internalStorage_getUInt32(const InternalStorageKey *key nonnull,
                                              uint32_t *valueIn in_parameter);

/** Deletes the key (and its value), \e STORAGE_ERROR_KEY_NOT_FOUND is returned if key was not found */
extern StorageError internalStorage_deleteKey(const InternalStorageKey *key nonnull);

extern bool internalStorage_hasKey(const InternalStorageKey *key nonnull);

extern StorageError internalStorage_openFile(const char *filePath nonnull,
                                             FILE **fileIn in_parameter);

extern StorageError internalStorage_closeFile(const FILE *fileIn nonnull);

extern StorageError internalStorage_readFileChunks(const FILE *file nonnull,
                                                   size_t startPosition,
                                                   void *bufferIn,
                                                   const uint bufferLength,
                                                   uint *bytesRead in_parameter nonnull);

extern StorageError internalStorage_readFile(const char *filePath nonnull,
                                             void *bufferIn nonnull in_parameter,
                                             const uint bufferLength,
                                             uint *bytesRead nonnull in_parameter);

extern StorageError internalStorage_writeFile(const char *filePath nonnull,
                                              const void *buffer nonnull in_parameter,
                                              const uint bufferLength,
                                              uint *bytesWritten nonnull in_parameter);

extern bool internalStorage_queryFileExists(const char *filePath nonnull);

extern StorageError internalStorage_queryFileSize(const char *filePath nonnull,
                                                  uint *fileSizeBytesIn nonnull in_parameter);

extern StorageError internalStorage_deleteFile(const char *filePath nonnull);

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGE_H
