#ifndef ESP32_REMOTECAMERA_LOGLIST_H
#define ESP32_REMOTECAMERA_LOGLIST_H

#include "Utils.h"
#include "List.h"

/**
 * An in-memory list containing log entries (lines),
 * uses List.h internally, however does not grow when capacity is filled,
 * instead, will loop around (like a ringbuffer)
 */
typedef void LogList;

typedef struct {
    unsigned int capacity;
} LogListOptions;

#define LOG_LIST_DEFAULT_CAPACITY 1000

#define LOG_LIST_DEFAULT_OPTIONS \
{.capacity=LOG_LIST_DEFAULT_CAPACITY,\
}

extern LogListOptions *logList_defaultOptions();

extern LogList *logList_create(const LogListOptions *options);

extern void logList_append(LogList *logList, const char *string);

extern ListError logList_getList(LogList *logList, List *listIn);

extern unsigned int logList_getCapacity(LogList *logList);

extern unsigned int logList_getSize(LogList *logList);

extern void logList_clear(LogList *logList);

extern void logList_destroy(LogList *logList);

#endif //ESP32_REMOTECAMERA_LOGLIST_H
