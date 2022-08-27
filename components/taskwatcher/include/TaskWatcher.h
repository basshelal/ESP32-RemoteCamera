#ifndef ESP32_REMOTECAMERA_TASKWATCHER_H
#define ESP32_REMOTECAMERA_TASKWATCHER_H

#include <stdbool.h>
#include <stdint.h>
#include "Constants.h"

typedef struct TaskInfo {
    const char *name;
    void (*startFunction)();
    uint32_t stackBytes;
} TaskInfo;

extern void taskWatcher_init();

extern void taskWatcher_loop();

extern void taskWatcher_registerTask(const TaskInfo *taskInfo);

extern void taskWatcher_deregisterTask(const char *taskName);

extern void taskWatcher_notifyTaskStarted(const char *taskName);

extern void taskWatcher_notifyTaskStopped(const char *taskName,
                                          const bool shouldRestart,
                                          const uint32_t remainingStackBytes);

#endif //ESP32_REMOTECAMERA_TASKWATCHER_H
