#ifndef ESP32_REMOTECAMERA_UTILS_H
#define ESP32_REMOTECAMERA_UTILS_H

#include <assert.h>

#define private static
#define public

#define attr(a) __attribute__((a))

#define EMPTY_MACRO_STATEMENT do{}while(0)

#define TODO(reason) __assert_func(__FILENAME__, __LINE__, NULL, reason)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define alloc(size) calloc(size, sizeof(char))
#define stralloc(strlen) alloc(((strlen) * sizeof(char)) + 1)
#define new(type) calloc(1, sizeof(type))

#define boolToString(boolValue) boolValue ? "true" : "false"
#define asString(codeName) #codeName

#define delayMillis(millis) vTaskDelay(pdMS_TO_TICKS(millis))

#endif //ESP32_REMOTECAMERA_UTILS_H
