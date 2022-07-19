#ifndef ESP32_REMOTECAMERA_UTILS_H
#define ESP32_REMOTECAMERA_UTILS_H
#include <assert.h>

#define private static
#define public
#define attr(a) __attribute__((a))
#define TODO(reason) __assert_func(__FILE_NAME__, __LINE__, NULL, reason)

#endif //ESP32_REMOTECAMERA_UTILS_H
