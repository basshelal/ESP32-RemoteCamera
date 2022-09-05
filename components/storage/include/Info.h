#ifndef ESP32_REMOTECAMERA_INFO_H
#define ESP32_REMOTECAMERA_INFO_H

#include <stddef.h>

typedef struct StorageInfo {
    char *mountPoint;
    uint64_t totalBytes;
    uint64_t usedBytes;
    uint64_t freeBytes;
} StorageInfo;

typedef struct FileInfo {
    uint64_t sizeBytes;
} FileInfo;

typedef struct DirInfo {
    uint64_t sizeBytes;
} DirInfo;

#endif //ESP32_REMOTECAMERA_INFO_H
