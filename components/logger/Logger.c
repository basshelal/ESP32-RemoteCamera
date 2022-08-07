#include <stdio.h>
#include <string.h>
#include "Logger.h"
#include "esp_log.h"

private struct {
    List *functionsList;
    LogList *logList;
} this;

private void logFunctionsListErrorCallback(const ListError listError) {

}

public void log_init() {
    ListOptions *listOptions = list_defaultListOptions();
    listOptions->errorCallback = logFunctionsListErrorCallback;
    this.functionsList = list_createWithOptions(listOptions);
    LogListOptions logListOptions = LOG_LIST_DEFAULT_OPTIONS;
    this.logList = logList_create(&logListOptions);
    INFO("Logger initialized");
}

public void log_addLogFunction(const LogFunction logFunction) {
    list_addItem(this.functionsList, logFunction);
}

public void log_removeLogFunction(const LogFunction logFunction) {
    list_removeItem(this.functionsList, logFunction);
}

public LogList *log_getLogList() {
    return this.logList;
}

public void log_log(const LogLevel logLevel, const char *tag, const char *format, const va_list vargs) {
    private char string[1024];
    vsprintf(string, format, vargs);
    ESP_LOG_LEVEL((esp_log_level_t) logLevel, tag, "%s", string);
    for (int i = 0; i < list_getSize(this.functionsList); i++) {
        LogFunction function = (LogFunction) list_getItem(this.functionsList, i);
        if (function != NULL) {
            function(logLevel, tag, string);
        }
    }
    logList_append(this.logList, string);
}

public void log_vargs(const LogLevel logLevel, const char *tag, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    log_log(logLevel, tag, format, vargs);
    va_end(vargs);
}