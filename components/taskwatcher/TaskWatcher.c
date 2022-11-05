#include "TaskWatcher.h"
#include "Utils.h"

#include <string.h>
#include "List.h"
#include "Logger.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    TaskInfo *taskInfo;
    TaskState state;
    bool shouldRestart;
} Task;

private struct {
    bool isInitialized;
    List *tasksList;
} this;

private void findTaskInList(const char *taskName, int *index, Task **taskResult) {
    for (int i = 0; i < list_getSize(this.tasksList); i++) {
        Task *task = list_getItem(this.tasksList, i);
        if (task != NULL && strcmp(task->taskInfo->name, taskName) == 0) {
            *index = i;
            *taskResult = task;
            break;
        }
    }
}

private bool isTaskInRTOS(const char *taskName) {
    TaskHandle_t handle = xTaskGetHandle(taskName);
    VERBOSE("Task \"%s\" was in RTOS?: %s", taskName, boolToString(handle != NULL));
    return handle != NULL;
}

public void taskWatcher_init() {
    if (!this.isInitialized) {
        ListOptions listOptions = LIST_DEFAULT_OPTIONS;
        listOptions.capacity = 10;
        listOptions.isGrowable = true;
        listOptions.errorCallback = NULL;
        this.tasksList = list_createWithOptions(&listOptions);

        this.isInitialized = true;
    }
}

public void taskWatcher_loop() {
    for (int i = 0; i < list_getSize(this.tasksList); i++) {
        Task *task = list_getItem(this.tasksList, i);
        if (task == NULL || task->taskInfo->name == NULL) continue;
        if (task->shouldRestart) {
            Error err = taskWatcher_startTask(task->taskInfo->name);
            if (err == ERROR_NONE) {
                task->shouldRestart = false;
            }
        }
    }
}

public Error taskWatcher_addTask(const TaskInfo *taskInfo) {
    Task *foundTask = NULL;
    int index = -1;
    findTaskInList(taskInfo->name, &index, &foundTask);
    if (foundTask != NULL) {
        throw(ERROR_ILLEGAL_ARGUMENT,
              "Cannot add task with name: \"%s\", a task with that name already exists",
              taskInfo->name);
    } else {
        Task *task = new(Task);
        task->taskInfo = new(TaskInfo);
        memcpy(task->taskInfo, taskInfo, sizeof(TaskInfo));
        task->state = TASK_STATE_STOPPED;
        task->shouldRestart = false;
        list_addItem(this.tasksList, task);
    }
    return ERROR_NONE;
}

public Error taskWatcher_removeTask(const char *taskName) {
    Task *task = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &task);
    if (task == NULL || index == -1) {
        throw(ERROR_ILLEGAL_ARGUMENT,
              "Cannot remove task \"%s\", no task with that name exists",
              taskName);
    } else {
        bool inRTOS = isTaskInRTOS(taskName);
        if (inRTOS || task->state != TASK_STATE_STOPPED) {
            throw(ERROR_ILLEGAL_STATE,
                  "Cannot remove task \"%s\", it has not been stopped correctly",
                  taskName);
        } else {
            list_removeItemIndexed(this.tasksList, index);
            delete(task->taskInfo);
            delete(task);
        }
    }
    return ERROR_NONE;
}

public Error taskWatcher_startTask(const char *taskName) {
    Task *task = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &task);
    if (task == NULL) {
        throw(ERROR_ILLEGAL_ARGUMENT, "Cannot start task \"%s\", no task with that name was found",
              taskName);
    }
    bool inRTOS = isTaskInRTOS(taskName);
    if (task->state == TASK_STATE_STOPPED && !inRTOS) {
        BaseType_t created = xTaskCreate(
                /*pvTaskCode=*/task->taskInfo->taskFunction,
                /*pcName=*/task->taskInfo->name,
                /*usStackDepth=*/task->taskInfo->stackBytes,
                /*pvParameters=*/task->taskInfo->taskParameter,
                /*uxPriority=*/task->taskInfo->taskPriority,
                /*pxCreatedTask=*/task->taskInfo->taskHandle
        );
        if (created != pdPASS) {
            throw(ERROR_LIBRARY_FAILURE, "FreeRTOS could not create task \"%s\", error code: %i",
                  taskName, created);
        }
        task->state = TASK_STATE_STARTED;
    } else {
        throw(ERROR_ILLEGAL_STATE,
              "Cannot start task \"%s\", task is not in stopped state", taskName);
    }
    return ERROR_NONE;
}

public Error taskWatcher_stopTask(const char *taskName) {
    Task *task = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &task);
    if (task == NULL) {
        throw(ERROR_ILLEGAL_ARGUMENT, "Cannot stop task \"%s\", no task with that name was found",
              taskName);
    }
    bool inRTOS = isTaskInRTOS(taskName);
    if (task->state != TASK_STATE_STARTED) {
        throw(ERROR_ILLEGAL_STATE,
              "Cannot stop task \"%s\", task is not in started state", taskName);
    }
    if (inRTOS) {
        vTaskDelete(task->taskInfo->taskHandle);
    }
    task->state = TASK_STATE_STOPPED;
    return ERROR_NONE;
}

public Error taskWatcher_restartTask(const char *taskName) {
    Task *task = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &task);
    if (task == NULL) {
        throw(ERROR_ILLEGAL_ARGUMENT, "Cannot restart task \"%s\", no task with that name was found",
              taskName);
    }
    Error err = taskWatcher_stopTask(taskName);
    if (err) return err;
    task->shouldRestart = true;
    return ERROR_NONE;
}

public Error taskWatcher_getTaskStackMinFreeBytes(const char *taskName, uint32_t *stackSizeBytes) {
    Task *task = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &task);
    if (task == NULL) {
        throw(ERROR_ILLEGAL_ARGUMENT, "No task found with name \"%s\"", taskName);
    }
    bool inRTOS = isTaskInRTOS(taskName);
    if (!inRTOS) {
        throw(ERROR_ILLEGAL_STATE, "Task \"%s\" was not found in RTOS, likely stopped", taskName);
    }
    *stackSizeBytes = uxTaskGetStackHighWaterMark(task->taskInfo->taskHandle);
    return ERROR_NONE;
}