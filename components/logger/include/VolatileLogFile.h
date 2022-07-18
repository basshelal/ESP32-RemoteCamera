#ifndef ESP32_REMOTECAMERA_VOLATILELOGFILE_H
#define ESP32_REMOTECAMERA_VOLATILELOGFILE_H

#include "Utils.h"
#include "List.h"

/**
 * An in-memory log file for quick logging, the speed and ease that this brings
 * comes at the cost of usability, this lives as long as the memory lives, hence it is
 * volatile
 */
typedef void VolatileLogFile;

typedef struct {
    unsigned int capacity;
} VolatileLogFileOptions;

#define VOLATILE_LOG_FILE_DEFAULT_CAPACITY 1000

extern VolatileLogFileOptions *volatileLogFile_defaultOptions();

extern VolatileLogFile *volatileLogFile_create(const VolatileLogFileOptions *options);

extern void volatileLogFile_append(VolatileLogFile *logFile, const char *string);

extern List *volatileLogFile_getList(VolatileLogFile *logFile);

extern ListError volatileLogFile_getListParameter(VolatileLogFile *logFile, List *listIn);

extern unsigned int volatileLogFile_capacity(VolatileLogFile *logFile);

extern unsigned int volatileLogFile_size(VolatileLogFile *logFile);

extern void volatileLogFile_clear(VolatileLogFile *logFile);

extern void volatileLogFile_destroy(VolatileLogFile *logFile);

#endif //ESP32_REMOTECAMERA_VOLATILELOGFILE_H
