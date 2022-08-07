#ifndef ESP32_REMOTECAMERA_LOGGER_H
#define ESP32_REMOTECAMERA_LOGGER_H

#include "Utils.h"
#include "List.h"
#include <esp_log.h>
#include "LogList.h"

typedef enum LogLevel {
    LOG_LEVEL_NONE = ESP_LOG_NONE,
    LOG_LEVEL_ERROR = ESP_LOG_ERROR,
    LOG_LEVEL_WARN = ESP_LOG_WARN,
    LOG_LEVEL_INFO = ESP_LOG_INFO,
    LOG_LEVEL_VERBOSE = ESP_LOG_VERBOSE
} LogLevel;

typedef void (*LogFunction)(const LogLevel logLevel, const char *tag, const char *string);

extern void log_init();

extern void log_addLogFunction(const LogFunction logFunction);

extern void log_removeLogFunction(const LogFunction logFunction);

extern LogList *log_getLogList();

extern void log_log(const LogLevel logLevel, const char *tag, const char *format, const va_list vargs);

__attribute__((format(printf, 3, 4)))
extern void log_vargs(const LogLevel logLevel, const char *tag, const char *format, ...);

#ifdef LOG_TAG
#define LOG_TEMPLATE(s, ...) LOG_TAG, "%s:%d "s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__
#else
#define LOG_TEMPLATE(s, ...) __FILENAME__, "%s:%d "s, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__
#endif

#define ERROR(message, ...) log_vargs(LOG_LEVEL_ERROR, LOG_TEMPLATE(message, ##__VA_ARGS__))
#define WARN(message, ...) log_vargs(LOG_LEVEL_WARN, LOG_TEMPLATE(message, ##__VA_ARGS__))
#define INFO(message, ...) log_vargs(LOG_LEVEL_INFO, LOG_TEMPLATE(message, ##__VA_ARGS__))
#define VERBOSE(message, ...) log_vargs(LOG_LEVEL_VERBOSE, LOG_TEMPLATE(message, ##__VA_ARGS__))

#endif //ESP32_REMOTECAMERA_LOGGER_H
