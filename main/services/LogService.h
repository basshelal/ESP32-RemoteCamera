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

extern void logE(const char *tag, const char *format, ...);

extern void logW(const char *tag, const char *format, ...);

extern void logI(const char *tag, const char *format, ...);

extern void logD(const char *tag, const char *format, ...);

extern void logV(const char *tag, const char *format, ...);

#endif //ESP32_REMOTECAMERA_LOGSERVICE_H
