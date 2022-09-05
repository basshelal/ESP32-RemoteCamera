#ifndef ESP32_REMOTECAMERA_CONSTANTS_H
#define ESP32_REMOTECAMERA_CONSTANTS_H

// TODO: 26-Aug-2022 @basshelal: Move all pins as #define constants here

// 0 to disable TaskWatcher, otherwise it is enabled
#define CONFIG_USE_TASK_WATCHER 1

// 0 to disable application level error toString functions that are used in error messages
//  these functions are always inlined and often have a large number of constant strings and are
//  a way to reduce overall binary size when disabled if needed at the cost of less clear error messages
#define CONFIG_USE_ERROR_TO_STRING 1

#endif //ESP32_REMOTECAMERA_CONSTANTS_H
