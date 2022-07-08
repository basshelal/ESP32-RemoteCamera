#include "LogService.h"
#include "List.h"
#include <esp_log.h>

private struct {
    List *logFunctionsList;
} logData;

private inline esp_log_level_t logLevelToEspLogLevel(const LogLevel logLevel) {
    // currently they line up perfectly so no need to do anything
    return (esp_log_level_t) logLevel;
}

private void logFunctionsListErrorCallback(const ListError listError) {

}

public void log_init() {
    ListOptions *listOptions = list_defaultListOptions();
    listOptions->errorCallback = logFunctionsListErrorCallback;
    logData.logFunctionsList = list_createOptions(listOptions);
}

public void log_addLogFunction(const LogFunction logFunction) {
    list_addItem(logData.logFunctionsList, logFunction);
}

public void log_removeLogFunction(const LogFunction logFunction) {
    list_removeItem(logData.logFunctionsList, logFunction);
}

public void log(const LogLevel logLevel, const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    // ESP_LOG_LEVEL();
    esp_log_writev(logLevelToEspLogLevel(logLevel), tag, format, vargs);
//    for (int i = 0; i < list_size(logData.logFunctionsList); i++) {
//        LogFunction function = (LogFunction) list_getItem(logData.logFunctionsList, i);
//        function(logLevel, tag, format, vargs);
//    }
    va_end(vargs);
}

public void logE(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log(ERROR, tag, format, vargs);
    va_end(vargs);
}

public void logW(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log(WARN, tag, format, vargs);
    va_end(vargs);
}

public void logI(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log(INFO, tag, format, vargs);
    va_end(vargs);
}

public void logD(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log(DEBUG, tag, format, vargs);
    va_end(vargs);
}

public void logV(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log(VERBOSE, tag, format, vargs);
    va_end(vargs);
}
