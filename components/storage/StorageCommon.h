#ifndef ESP32_REMOTECAMERA_STORAGECOMMON_H
#define ESP32_REMOTECAMERA_STORAGECOMMON_H

#include <stddef.h>
#include "include/Storage.h"

extern inline const char *fileAccessModeToString(const FileAccessMode accessMode) {
    switch (accessMode) {
        case READ:
            return "r";
        case WRITE:
            return "w";
        case APPEND:
            return "a";
        case READ_EXTENDED:
            return "r+";
        case WRITE_EXTENDED:
            return "w+";
        case APPEND_EXTENDED:
            return "a+";
        default:
            return NULL;
    }
}

#endif //ESP32_REMOTECAMERA_STORAGECOMMON_H
