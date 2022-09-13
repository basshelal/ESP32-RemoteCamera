#ifndef ESP32_REMOTECAMERA_INFO_H
#define ESP32_REMOTECAMERA_INFO_H

#include <stddef.h>

typedef struct StorageInfo {
    char *mountPoint;
    uint32_t totalBytes;
    uint32_t usedBytes;
    uint32_t freeBytes;
} StorageInfo;

typedef struct FileInfo {
    uint32_t sizeBytes;
} FileInfo;

typedef struct DirInfo {
    uint32_t sizeBytes;
} DirInfo;

#endif //ESP32_REMOTECAMERA_INFO_H
