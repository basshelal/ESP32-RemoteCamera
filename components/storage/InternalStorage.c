#include "InternalStorage.h"

typedef void InternalStorageKey;

#define undeclared static

undeclared StorageError internalStorage_init() {

    return STORAGE_ERROR_NONE;
}

// Put if not exists or set if exists, check exists by calling get
undeclared StorageError internalStorage_putKeyValue(const InternalStorageKey *key, const void *value) {

    return STORAGE_ERROR_NONE;
}

// Get, if not exists valueIn is NULL and an error is returned
undeclared StorageError internalStorage_getKeyValue(const InternalStorageKey *key, void *valueIn) {

    return STORAGE_ERROR_NONE;
}

// Delete if exists, error if not
undeclared StorageError internalStorage_deleteKeyValue(const InternalStorageKey *key) {

    return STORAGE_ERROR_NONE;
}

undeclared StorageError internalStorage_getAllKeyValue(void *valuesIn) {
    // TODO: 18-Jul-2022 @basshelal: Is this possible? If so see what's more efficient 1 call with array alloc in
    //  place and size as an int pointer param or 2 calls with a size function and a getAll function limited by size
    //  because pointer is pre-allocated
    return STORAGE_ERROR_NONE;
}

// Put if not exists or set if exists, check exists by calling get
undeclared StorageError internalStorage_putFile(const char *filePath, const FILE *file) {
    return STORAGE_ERROR_NONE;
}

// Get, if not exists fileIn is NULL and error is returned
undeclared StorageError internalStorage_getFile(const char *filePath, FILE *fileIn) {
    return STORAGE_ERROR_NONE;
}

// Delete if exists, error if not
undeclared StorageError internalStorage_deleteFile(const char *filePath) {
    return STORAGE_ERROR_NONE;
}