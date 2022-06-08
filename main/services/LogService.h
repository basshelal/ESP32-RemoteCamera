#ifndef ESP32_REMOTECAMERA_LOGSERVICE_H
#define ESP32_REMOTECAMERA_LOGSERVICE_H

#include "utils.h"

typedef enum LogLevel {
    NONE, ERROR, WARN, INFO, DEBUG, VERBOSE
} LogLevel;

typedef void (*LogFunction)(const LogLevel logLevel, const char *tag, const char *format, ...);

extern void log_init();

extern void log_addLogFunction(const LogFunction logFunction);

extern void log_removeLogFunction(const LogFunction logFunction);

extern void log(const LogLevel logLevel, const char *tag, const char *format, ...);

extern void logE();

extern void logW();

extern void logI();

extern void logD();

extern void logV();

#endif //ESP32_REMOTECAMERA_LOGSERVICE_H
