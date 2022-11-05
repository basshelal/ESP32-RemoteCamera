#ifndef ESP32_REMOTECAMERA_TASKWATCHER_H
#define ESP32_REMOTECAMERA_TASKWATCHER_H

#include "Constants.h"
#include "Error.h"
#include <stdbool.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

typedef struct TaskInfo_t {
    const char *name;
    TaskFunction_t taskFunction;
    uint32_t stackBytes;
    void *const taskParameter;
    UBaseType_t taskPriority;
    TaskHandle_t *const taskHandle;
} TaskInfo;

typedef enum TaskState_t {
    TASK_STATE_STOPPED,
    TASK_STATE_STARTED,
} TaskState;

extern void taskWatcher_init();

extern void taskWatcher_loop();

extern Error taskWatcher_addTask(const TaskInfo *taskInfo);

extern Error taskWatcher_removeTask(const char *taskName);

extern Error taskWatcher_startTask(const char *taskName);

extern Error taskWatcher_stopTask(const char *taskName);

extern Error taskWatcher_restartTask(const char *taskName);

extern Error taskWatcher_getTaskStackMinFreeBytes(const char *taskName, uint32_t *stackSizeBytes);

#endif //ESP32_REMOTECAMERA_TASKWATCHER_H
