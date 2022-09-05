#include "TaskWatcher.h"
#include "Utils.h"

#if CONFIG_USE_TASK_WATCHER

#include <string.h>
#include "List.h"
#include "Logger.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    TaskInfo taskInfo;
    enum {
        INVALID, STARTED, STOPPED, SHOULD_RESTART, RESTARTING
    } state;
} Task;

private struct {
    bool isInitialized;
    List *tasksList;
} this;

private void tasksListErrorCallback(const ListError error) {

}

private void findTaskInList(const char *taskName, int *index, Task **taskResult) {
    for (int i = 0; i < list_getSize(this.tasksList); i++) {
        Task *task = list_getItem(this.tasksList, i);
        if (task != NULL && strcmp(task->taskInfo.name, taskName) == 0) {
            *index = i;
            *taskResult = task;
            break;
        }
    }
}

private bool isTaskInRTOS(const char *taskName) {
    TaskHandle_t handle = xTaskGetHandle(taskName);
    INFO("Task \"%s\" was %s", taskName, boolToString(handle != NULL));
    return handle != NULL;
}

public void taskWatcher_init() {
    if (!this.isInitialized) {
        ListOptions listOptions = LIST_DEFAULT_OPTIONS;
        listOptions.capacity = 10;
        listOptions.isGrowable = true;
        listOptions.errorCallback = tasksListErrorCallback;
        this.tasksList = list_createWithOptions(&listOptions);

        this.isInitialized = true;
    }
}

public void taskWatcher_loop() {
    for (int i = 0; i < list_getSize(this.tasksList); i++) {
        Task *task = list_getItem(this.tasksList, i);
        if (task != NULL && task->state == SHOULD_RESTART &&
            task->taskInfo.name != NULL &&
            !isTaskInRTOS(task->taskInfo.name)) {
            if (task->taskInfo.startFunction) {
                INFO("Restarting \"%s\"", task->taskInfo.name);
                task->state = RESTARTING;
                task->taskInfo.startFunction();
            }
        }
    }
}

public void taskWatcher_registerTask(const TaskInfo *taskInfo) {
    Task *foundTask = NULL;
    int index = -1;
    findTaskInList(taskInfo->name, &index, &foundTask);
    if (foundTask != NULL) {
        ERROR("Cannot register task, a task already exists with the name: %s", taskInfo->name);
    } else {
        Task *task = new(Task);
        task->taskInfo = *taskInfo;
        task->state = INVALID;
        list_addItem(this.tasksList, task);
    }
}

public void taskWatcher_deregisterTask(const char *taskName) {
    Task *foundTask = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &foundTask);
    if (foundTask != NULL && index != -1) {
        list_removeItemIndexed(this.tasksList, index);
    }
}

public void taskWatcher_notifyTaskStarted(const char *taskName) {
    Task *foundTask = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &foundTask);
    if (foundTask != NULL) {
        foundTask->state = STARTED;
    }
}

public void taskWatcher_notifyTaskStopped(const char *taskName,
                                          const bool shouldRestart,
                                          const uint32_t remainingStackBytes) {
    Task *foundTask = NULL;
    int index = -1;
    findTaskInList(taskName, &index, &foundTask);
    if (foundTask != NULL) {
        if (shouldRestart) {
            WARN("Task \"%s\" stopped with %u remaining / %u total bytes and will restart",
                 foundTask->taskInfo.name,
                 remainingStackBytes,
                 foundTask->taskInfo.stackBytes);
            foundTask->state = SHOULD_RESTART;
        } else {
            INFO("Task \"%s\" stopped successfully", foundTask->taskInfo.name);
            foundTask->state = STOPPED;
        }
    }
}

#else

public void taskWatcher_init() {
}

public void taskWatcher_loop() {
}

public void taskWatcher_registerTask(const TaskInfo *taskInfo) {
}

public void taskWatcher_deregisterTask(const char *taskName) {
}

public void taskWatcher_notifyTaskStarted(const char *taskName) {
}

public void taskWatcher_notifyTaskStopped(const char *taskName,
                                          const bool shouldRestart,
                                          const uint32_t remainingStackBytes) {
}

#endif // CONFIG_USE_TASK_WATCHER