#include "LogList.h"
#include <stdlib.h>
#include "List.h"

typedef struct {
    List *list;
    LogListOptions options;
    index_t nextWriteIndex;
} LogListData;

public LogList *logList_create(const LogListOptions *options) {
    LogListData *this = new(LogListData);
    LogListOptions logListOptions = LOG_LIST_DEFAULT_OPTIONS;
    if (options != NULL) {
        logListOptions = *options;
    }
    this->options = logListOptions;
    ListOptions listOptions = LIST_DEFAULT_OPTIONS;
    listOptions.isGrowable = false;
    listOptions.capacity = (int) this->options.capacity;
    listOptions.errorCallback = NULL;
    this->list = list_createWithOptions(&listOptions);
    this->nextWriteIndex = 0;
    return this;
}

public void logList_destroy(LogList *logList) {
    if (!logList) return;
    LogListData *this = (LogListData *) logList;
    list_destroy(this->list);
    free(this);
}

public LogListOptions *logList_defaultOptions() {
    LogListOptions *options = new(LogListOptions);
    options->capacity = LOG_LIST_DEFAULT_CAPACITY;
    return options;
}

public void logList_append(LogList *logList, const char *string) {
    if (!logList || !string) return;
    LogListData *this = (LogListData *) logList;
    list_setItem(this->list, this->nextWriteIndex, string);
    this->nextWriteIndex++;
    if (this->nextWriteIndex >= this->options.capacity) {
        this->nextWriteIndex = 0;
    }
}

public ListError logList_getList(LogList *logList, List *listIn) {
    if (!logList || !listIn) return LIST_ERROR_NULL_LIST;
    LogListData *this = (LogListData *) logList;
    ListError err;
    err = list_clear(listIn);
    if (err) return err;
    const capacity_t size = logList_getSize(this);
    const capacity_t capacity = logList_getCapacity(this);
    if (size < capacity) {
        for (index_t index = 0; index < size; index++) {
            err = list_addItem(listIn, list_getItem(this->list, index));
            if (err) return err;
        }
    } else if (size == capacity) {
        for (index_t index = this->nextWriteIndex; index < capacity; index++) {
            err = list_addItem(listIn, list_getItem(this->list, index));
            if (err) return err;
        }
        for (index_t index = 0; index < this->nextWriteIndex; index++) {
            err = list_addItem(listIn, list_getItem(this->list, index));
            if (err) return err;
        }
    }
    return LIST_ERROR_NONE;
}

public unsigned int logList_getCapacity(LogList *logList) {
    if (!logList) return LIST_INVALID_INDEX_CAPACITY;
    LogListData *this = (LogListData *) logList;
    return this->options.capacity;
}

public unsigned int logList_getSize(LogList *logList) {
    if (!logList) return LIST_INVALID_INDEX_CAPACITY;
    LogListData *this = (LogListData *) logList;
    return list_getSize(this->list);
}

public void logList_clear(LogList *logList) {
    if (!logList) return;
    LogListData *this = (LogListData *) logList;
    list_clear(this->list);
    this->nextWriteIndex = 0;
}