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

extern void log_log(const LogLevel logLevel, const char *tag, const char *format, const va_list vargs);

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

#ifdef LOG_TAG
#define LOG_TEMPLATE(s, ...) LOG_TAG, "%s:%d "s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__
#else
#define LOG_TEMPLATE(s, ...) __FILE__, "%s:%d "s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__
#endif

#define ERROR(message, ...) logE(LOG_TEMPLATE(message, ##__VA_ARGS__))
#define WARN(message, ...) logW(LOG_TEMPLATE(message, ##__VA_ARGS__))
#define INFO(message, ...) logI(LOG_TEMPLATE(message, ##__VA_ARGS__))
#define DEBUG(message, ...) logD(LOG_TEMPLATE(message, ##__VA_ARGS__))
#define VERBOSE(message, ...) logV(LOG_TEMPLATE(message, ##__VA_ARGS__))

#endif //ESP32_REMOTECAMERA_LOGGER_H
