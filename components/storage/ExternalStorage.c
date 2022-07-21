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