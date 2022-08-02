#ifndef ESP32_REMOTECAMERA_FILEMODE_H
#define ESP32_REMOTECAMERA_FILEMODE_H

#include "Utils.h"

typedef enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_READ_EXT,
    FILE_MODE_WRITE_EXT,
    FILE_MODE_APPEND_EXT,
} FileMode;

public inline const char *fileModeToString(const FileMode fileMode) {
    switch (fileMode) {
        case FILE_MODE_READ:
            return "r";
        case FILE_MODE_WRITE:
            return "w";
        case FILE_MODE_APPEND:
            return "a";
        case FILE_MODE_READ_EXT:
            return "r+";
        case FILE_MODE_WRITE_EXT:
            return "w+";
        case FILE_MODE_APPEND_EXT:
            return "a+";
        default:
            return NULL;
    }
}

#endif //ESP32_REMOTECAMERA_FILEMODE_H
