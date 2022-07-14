#include "utils.h"
#include "services/LogService.h"
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

__attribute__((__noreturn__, __used__))
public void app_main() {
    setup();
    while (true) {
        battery_read();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
