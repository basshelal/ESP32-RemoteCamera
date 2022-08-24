#ifndef ESP32_REMOTECAMERA_INFO_H
#define ESP32_REMOTECAMERA_INFO_H

#include <stddef.h>

typedef struct StorageInfo {

} StorageInfo;

typedef struct SDCardInfo {
    char *name;
    uint32_t speed;
    size_t capacity;
    size_t size;
} SDCardInfo;

typedef struct FileInfo {
    size_t size;
} FileInfo;

typedef struct DirInfo {

} DirInfo;

#endif //ESP32_REMOTECAMERA_INFO_H
