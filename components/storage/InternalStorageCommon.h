#ifndef ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H
#define ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H

#include "Utils.h"

#define LOG_TAG "InternalStorage:"__FILE__
#define undeclared static

#define throw(error, message, ...) \
ERROR(message, ##__VA_ARGS__);   \
return error

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#endif //ESP32_REMOTECAMERA_INTERNALSTORAGECOMMON_H
