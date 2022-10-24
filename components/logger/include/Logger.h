#ifndef ESP32_REMOTECAMERA_LOGGER_H
#define ESP32_REMOTECAMERA_LOGGER_H

#include "Constants.h"
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
#define LOG_TEMPLATE(message, ...)\
LOG_TAG, "%s:%d "message, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__
#else
#define LOG_TEMPLATE(message, ...)\
__FILENAME__, "%s:%d "message, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__
#endif

#if CONFIG_LOGGER_LOG_LEVEL >= CONFIG_LOG_LEVEL_ERROR
#define ERROR(message, ...) log_vargs(LOG_LEVEL_ERROR, LOG_TEMPLATE(message, ##__VA_ARGS__))
#else
#define ERROR(message, ...) EMPTY_MACRO_STATEMENT
#endif

#if CONFIG_LOGGER_LOG_LEVEL >= CONFIG_LOG_LEVEL_WARN
#define WARN(message, ...) log_vargs(LOG_LEVEL_WARN, LOG_TEMPLATE(message, ##__VA_ARGS__))
#else
#define WARN(message, ...) EMPTY_MACRO_STATEMENT
#endif

#if CONFIG_LOGGER_LOG_LEVEL >= CONFIG_LOG_LEVEL_INFO
#define INFO(message, ...) log_vargs(LOG_LEVEL_INFO, LOG_TEMPLATE(message, ##__VA_ARGS__))
#else
#define INFO(message, ...) EMPTY_MACRO_STATEMENT
#endif

#if CONFIG_LOGGER_LOG_LEVEL >= CONFIG_LOG_LEVEL_VERBOSE
#define VERBOSE(message, ...) log_vargs(LOG_LEVEL_VERBOSE, LOG_TEMPLATE(message, ##__VA_ARGS__))
#else
#define VERBOSE(message, ...) EMPTY_MACRO_STATEMENT
#endif

#define throw(error, message, ...) \
ERROR(message, ##__VA_ARGS__);   \
return error

#define throwIfError(func, message, ...) \
do{                                      \
int error = func;\
if (error != 0) {                                     \
throw(error, message, ##__VA_ARGS__);                \
}}while(0)

#define throwESPError(functionName, errno) \
throw(ERROR_LIBRARY_FAILURE, #functionName" returned: %i: %s", err, esp_err_to_name(err))

#define throwLibCError(functionName, errno) \
throw(ERROR_LIBRARY_FAILURE, #functionName" returned: %i: %s", err, strerror(err))

#define throwLibCErrorMessage(functionName, errno, message, ...) \
throw(ERROR_LIBRARY_FAILURE, #functionName" returned: %i: %s " message, err, strerror(err), ##__VA_ARGS__)

#define require(condition, error, message, ...) \
do{                                                  \
if (!(condition)) {                                  \
throw(error, message, ##__VA_ARGS__);                \
}}while(0)

#define requireNotNull(pointer, error, message, ...) require(pointer != NULL, error, message, ##__VA_ARGS__)

#define requireArgNotNull(element) \
requireNotNull(element, ERROR_NULL_ARGUMENT, #element" cannot be NULL")

#endif //ESP32_REMOTECAMERA_LOGGER_H
