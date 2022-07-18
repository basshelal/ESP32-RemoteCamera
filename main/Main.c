#include <esp_log.h>
#include "Utils.h"
#include "Logger.h"
#include "services/WifiService.h"
#include "services/WebServer.h"
#include "services/DataStore.h"
#include "services/Camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "services/Battery.h"

private void setup() {
    log_init();
    wifi_init();
    wifi_connect(WIFI_MODE_STA);
    webServer_init();
    dataStore_init();
    camera_init();
    battery_init();
}

attr(__used__) attr(__noreturn__)
public void app_main() {
    setup();
    while (true) {
        logI("Battery", "%s", battery_text());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
