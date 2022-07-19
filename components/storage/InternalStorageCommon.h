#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H

#include "Utils.h"

#define LOG_TAG "InternalStorage:"__FILE_NAME__
#define undeclared static

#define throw(error, message, ...) \
ERROR(message, ##__VA_ARGS__);   \
return error

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H
