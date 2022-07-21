#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGE_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGE_H

#include "Utils.h"
#include "StorageError.h"
#include <stdio.h>
#include <stdbool.h>

typedef char InternalStorageKey;

extern StorageError internalStorage_init();

/** Destroys and de-initializes all internal structures,
 * call only when you know a restart or shutdown will definitely happen and internal storage will no longer be used */
extern StorageError internalStorage_destroy();

/** Puts the value at key, updating it if it exists and creating it if it doesn't,
 * you can check a key's existence using \c internalStorage_hasKey */
extern StorageError internalStorage_putString(const InternalStorageKey *key, const char *value);

/** Get the string at the key, valueIn will contain the result, pass in a NULL ptr and call free
 * when done with it as this is allocated in this function.
 * If no value is in key then valueIn is unchanged and \e STORAGE_ERROR_KEY_NOT_FOUND is returned*/
extern StorageError internalStorage_getString(const InternalStorageKey *key,
                                              char *valueIn nonnull in_parameter);

/** Deletes the key (and its value), \e STORAGE_ERROR_KEY_NOT_FOUND is returned if key was not found */
extern StorageError internalStorage_deleteKey(const InternalStorageKey *key nonnull);

extern bool internalStorage_hasKey(const InternalStorageKey *key nonnull);

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
