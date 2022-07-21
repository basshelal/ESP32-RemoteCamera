#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"

__attribute__((__noreturn__, __used__))
void app_main() {
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
