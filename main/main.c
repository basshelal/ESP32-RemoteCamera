#include "utils.h"
#include "services/LogService.h"
#include "services/WifiService.h"
#include "services/WebServer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

private void logFunction(const LogLevel logLevel, const char *tag, const char *format, ...) {
//    va_list vargs;
//    va_start(vargs, format);
//    fprintf(stderr, format, vargs);
//    va_end(vargs);
}

private void setup() {
    log_init();
    WifiService.init();
    WifiService.connect(WIFI_MODE_STA);
    WebServer.init();
    log_addLogFunction(logFunction);
    log(INFO, "MAIN", "My test logging statement %i %i %i", 1, 2, 3);
    //logI("MAIN", "Finished setup, one:");
}

__attribute__((__noreturn__, __used__))
public void app_main() {
    setup();
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
