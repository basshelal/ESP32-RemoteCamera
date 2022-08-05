#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "Utils.h"

attr(__used__) attr(__noreturn__)
public void app_main() {
    UNITY_BEGIN();
    unity_run_all_tests();
//    printf("%u\n", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
//    printf("%u\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    UNITY_END();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
