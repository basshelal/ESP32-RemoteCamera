#include "Constants.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "Utils.h"
#include "Logger.h"

attr(__used__) attr(__noreturn__)
public void app_main() {
    log_init();
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
