#include "Constants.h"
#include "Utils.h"
#include "Logger.h"
#include "Wifi.h"
#include "Webserver.h"
#include "Battery.h"
#include "ExternalStorage.h"
#include "TaskWatcher.h"
#include "services/Camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

private void setup() {
    // The order of the below matter!
    // Generally, it's like a dependency graph, where the first one is the one that depends on no-one
    // and is depended on by all, and the last one is depended on by no-one but depends on many of the above
    log_init();
    taskWatcher_init();
    wifi_init();
    wifi_connect(WIFI_MODE_STA);
    webserver_init();
    ExternalStorageOptions externalStorageOptions = EXTERNAL_STORAGE_DEFAULT_OPTIONS;
    externalStorageOptions.startAutoMountTask = true;
    externalStorage_init(&externalStorageOptions);
    camera_init();
    battery_init();
    camera_read();
}

attr(__used__) attr(__noreturn__)
public void app_main() {
    setup();
    while (true) {
        taskWatcher_loop();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
