#include <stdio.h>
#include "Logger.h"
#include "esp_log.h"

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

public void log_log(const LogLevel logLevel, const char *tag, const char *format, const va_list vargs) {
    private char string[1024];
    vsprintf(string, format, vargs);
    ESP_LOG_LEVEL(logLevelToEspLogLevel(logLevel), tag, "%s", string);
    for (int i = 0; i < list_size(logData.logFunctionsList); i++) {
        LogFunction function = (LogFunction) list_getItem(logData.logFunctionsList, i);
        if (function != NULL) {
            function(logLevel, tag, string);
        }
    }
}

public void logE(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log_log(ERROR, tag, format, vargs);
    va_end(vargs);
}

public void logW(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log_log(WARN, tag, format, vargs);
    va_end(vargs);
}

public void logI(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log_log(INFO, tag, format, vargs);
    va_end(vargs);
}

public void logD(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log_log(DEBUG, tag, format, vargs);
    va_end(vargs);
}

public void logV(const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log_log(VERBOSE, tag, format, vargs);
    va_end(vargs);
}