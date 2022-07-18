#include "VolatileLogFile.h"
#include <stdlib.h>
#include "List.h"

typedef struct {
    List *list;
    unsigned int capacity;
    unsigned int size;
    unsigned int writeIndex;
} LogFileData;

public VolatileLogFile *volatileLogFile_create(const VolatileLogFileOptions *options) {
    LogFileData *file = malloc(sizeof(LogFileData));
    ListOptions *listOptions = list_defaultListOptions();
    listOptions->isGrowable = false;
    listOptions->capacity = (int) options->capacity;
    file->list = list_createOptions(listOptions);
    file->capacity = options->capacity;
    file->size = 0;
    file->writeIndex = 0;
    return file;
}

public void volatileLogFile_destroy(VolatileLogFile *logFile) {
    LogFileData *file = (LogFileData *) logFile;
    list_destroy(file->list);
    free(file);
}

extern VolatileLogFileOptions *volatileLogFile_defaultOptions() {
    VolatileLogFileOptions *options = calloc(1, sizeof(VolatileLogFileOptions));
    VolatileLogFileOptions o = {
            .capacity = VOLATILE_LOG_FILE_DEFAULT_CAPACITY
    };
    *options = o;
    return options;
}

public void volatileLogFile_append(VolatileLogFile *logFile, const char *string) {
    LogFileData *file = (LogFileData *) logFile;
    // TODO: 17-Jul-2022 @basshelal: Handle the case when writeIndex has and/or will exceed size
}

public List *volatileLogFile_getList(VolatileLogFile *logFile) {
    LogFileData *file = (LogFileData *) logFile;
    ListOptions *listOptions = list_defaultListOptions();
    listOptions->isGrowable = false;
    listOptions->capacity = (int) file->size;
    List *result = list_createOptions(listOptions);
    // TODO: 17-Jul-2022 @basshelal: Fill result with list in file but with correct loop around
    return result;
}

public ListError volatileLogFile_getListParameter(VolatileLogFile *logFile, List *listIn) {
    LogFileData *file = (LogFileData *) logFile;
    // TODO: 17-Jul-2022 @basshelal: Modify listIn, returning any errors faced back to the caller
    return LIST_ERROR_NONE;
}

public unsigned int volatileLogFile_capacity(VolatileLogFile *logFile) {
    LogFileData *file = (LogFileData *) logFile;
    return file->capacity;
}

public unsigned int volatileLogFile_size(VolatileLogFile *logFile) {
    LogFileData *file = (LogFileData *) logFile;
    return file->size;
}

public void volatileLogFile_clear(VolatileLogFile *logFile) {
    LogFileData *file = (LogFileData *) logFile;
    list_clear(file->list);
    file->size = 0;
    file->writeIndex = 0;
}