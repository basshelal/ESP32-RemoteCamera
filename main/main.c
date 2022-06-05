#include "utils.h"
#include "services/WifiService.h"
#include "services/WebServer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

private void setup() {
    WifiService.init();
    WifiService.connect(WIFI_MODE_STA);
    WebServer.init();
}

__attribute__((__noreturn__, __used__))
public void app_main() {
    setup();
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
