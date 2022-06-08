#include "LogService.h"
#include "../utils/List.h"
#include <esp_log.h>

private struct {
    List *logFunctionsList;
} logData;

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

}

public void logE() {

}

public void logW() {

}

public void logI() {

}

public void logD() {

}

public void logV() {

}
