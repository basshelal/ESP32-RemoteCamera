#include "Utils.h"
#include "Logger.h"
#include "Wifi.h"
#include "Webserver.h"
#include "services/DataStore.h"
#include "services/Camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "services/Battery.h"

private void setup() {
    log_init();
    wifi_init();
    wifi_connect(WIFI_MODE_STA);
    webserver_init();
    dataStore_init();
    camera_init();
    battery_init();
    camera_read();
}

attr(__used__) attr(__noreturn__)
public void app_main() {
    setup();
    while (true) {
        INFO("%s", battery_text());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
