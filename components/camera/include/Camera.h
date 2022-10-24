#ifndef ESP32_REMOTECAMERA_CAMERA_H
#define ESP32_REMOTECAMERA_CAMERA_H

#include "Error.h"

extern Error camera_init();

extern Error camera_destroy();

typedef void CameraReadCallback(char *buffer, int bufferSize, void *userArgs);

extern Error camera_readImage(const int chunkSize, CameraReadCallback readCallback, void *userArg);

#endif //ESP32_REMOTECAMERA_CAMERA_H
