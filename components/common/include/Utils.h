#ifndef ESP32_REMOTECAMERA_UTILS_H
#define ESP32_REMOTECAMERA_UTILS_H
#include <assert.h>

#define private static
#define public

#define attr(a) __attribute__((a))
#define nonnull attr()
#define returns_nonnull attr(__returns_nonnull__)
#define in_parameter attr()

#define TODO(reason) __assert_func(__FILE__, __LINE__, NULL, reason)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#endif //ESP32_REMOTECAMERA_UTILS_H