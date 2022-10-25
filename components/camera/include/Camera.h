#ifndef ESP32_REMOTECAMERA_CAMERA_H
#define ESP32_REMOTECAMERA_CAMERA_H

#include "Error.h"

typedef enum CameraImageSize {
    CAMERA_IMAGE_SIZE_DEFAULT,
    CAMERA_IMAGE_SIZE_320x240,
    CAMERA_IMAGE_SIZE_640x480,
    CAMERA_IMAGE_SIZE_1024x768,
    CAMERA_IMAGE_SIZE_1280x960,
    CAMERA_IMAGE_SIZE_1600x1200,
    CAMERA_IMAGE_SIZE_2048x1536,
    CAMERA_IMAGE_SIZE_2592x1944,
} CameraImageSize;

typedef void CameraReadCallback(char *buffer, int bufferSize, void *userArgs);

extern Error camera_init();

extern Error camera_destroy();

extern Error camera_captureImage(uint32_t *imageSize);

extern Error camera_setImageSize(const CameraImageSize imageSize);

extern Error camera_readImage(char *buffer, const size_t bufferLength);

extern Error camera_readImageWithCallback(const int chunkSize, CameraReadCallback readCallback, void *userArg);

#endif //ESP32_REMOTECAMERA_CAMERA_H
