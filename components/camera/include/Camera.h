#ifndef ESP32_REMOTECAMERA_CAMERA_H
#define ESP32_REMOTECAMERA_CAMERA_H

#include "Error.h"
#include <stdbool.h>

typedef enum CameraImageSize {
    CAMERA_IMAGE_SIZE_320x240 = 0,
    CAMERA_IMAGE_SIZE_640x480 = 1,
    CAMERA_IMAGE_SIZE_1024x768 = 2,
    CAMERA_IMAGE_SIZE_1280x960 = 3,
    CAMERA_IMAGE_SIZE_1600x1200 = 4,
    CAMERA_IMAGE_SIZE_2048x1536 = 5,
    CAMERA_IMAGE_SIZE_2592x1944 = 6,
    CAMERA_IMAGE_SIZE_DEFAULT = CAMERA_IMAGE_SIZE_1280x960
} CameraImageSize;

typedef void CameraReadCallback(char *buffer, int bufferSize, void *userArgs);

typedef void CameraLiveCaptureCallback(uint8_t *buffer, size_t bufferLength,
                                       uint32_t totalImageSize);

extern Error camera_init();

/** Initializes the camera to its default values ready to use, can be called multiple times to restart camera */
extern Error camera_start();

extern Error camera_pauseLiveCapture(bool pause);

extern Error camera_destroy();

extern Error camera_captureImage(uint32_t *imageSize);

extern Error camera_setImageSize(const CameraImageSize imageSize);

extern Error camera_setCameraLiveCaptureCallback(CameraLiveCaptureCallback cameraLiveCaptureCallback);

extern Error camera_readImageBufferedWithCallback(char *buffer, const int bufferLength,
                                                  const uint32_t imageSize,
                                                  CameraReadCallback readCallback, void *userArg);

#endif //ESP32_REMOTECAMERA_CAMERA_H
