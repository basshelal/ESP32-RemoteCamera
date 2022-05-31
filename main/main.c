#include "utils.h"
#include "services/wifi.h"
#include "services/webserver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

private void setup() {
    wifi_init();
    webServer_init();
}

private void loop() {

}

public void app_main() {
    setup();
    vTaskDelay(portMAX_DELAY);
}
