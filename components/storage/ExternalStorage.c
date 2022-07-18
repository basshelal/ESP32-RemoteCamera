#include "ExternalStorage.h"

#define undeclared static

undeclared StorageError externalStorage_init() {
    return STORAGE_ERROR_NONE;
}

undeclared bool externalStorage_hasCard() {
    return false;
}

undeclared StorageError externalStorage_mount() {
    return STORAGE_ERROR_NONE;
}

// TODO: 18-Jul-2022 @basshelal: Unsure about API here, if we implement it well we need a lot of functions,
//  it all depends on the POSIX and ESP-IDF file API

undeclared StorageError externalStorage_getFile(const char *filePath, FILE *fileIn) {
    return STORAGE_ERROR_NONE;
}