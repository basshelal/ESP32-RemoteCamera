#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H

#include "Utils.h"

#define LOG_TAG "InternalStorage:"__FILE__
#define undeclared static

#define throw(error, message, ...) \
ERROR(message, ##__VA_ARGS__);   \
return error

#define requireNotNull(pointer, error, message, ...) \
if (pointer == NULL) {                               \
throw(error, message, ##__VA_ARGS__);                \
}

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H
