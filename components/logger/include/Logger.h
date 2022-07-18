#ifndef ESP32_REMOTECAMERA_LOGGER_H
#define ESP32_REMOTECAMERA_LOGGER_H

#include "Utils.h"
#include "List.h"

#define __log_function__ __attribute__((format(printf, 2, 3)))

typedef enum LogLevel {
    NONE, ERROR, WARN, INFO, DEBUG, VERBOSE
} LogLevel;

typedef void (*LogFunction)(const LogLevel logLevel, const char *tag, const char *string);

extern void log_init();

extern void log_addLogFunction(const LogFunction logFunction);

extern void log_removeLogFunction(const LogFunction logFunction);

extern void log(const LogLevel logLevel, const char *tag, const char *format, const va_list vargs);

__log_function__
extern void logE(const char *tag, const char *format, ...);

__log_function__
extern void logW(const char *tag, const char *format, ...);

__log_function__
extern void logI(const char *tag, const char *format, ...);

__log_function__
extern void logD(const char *tag, const char *format, ...);

__log_function__
extern void logV(const char *tag, const char *format, ...);

#undef __log_function__

#endif //ESP32_REMOTECAMERA_LOGGER_H
