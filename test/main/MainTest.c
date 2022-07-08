#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "List.h"

TEST_CASE("My test", "Description") {
    printf("My test case has ran!\n");
    printf("My test case has ran!\n");
    printf("My test case has ran!\n");
    printf("My test case has ran!\n");
}

__attribute__((__noreturn__, __used__))
void app_main() {

    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");
    printf("Hello from Test!\n");

//    List *list = list_create();
//
//    list_addItem(list, "My first item");
//    list_addItem(list, "My second item");
//    list_addItem(list, "My third item");
//
//    for (int i = 0; i < list_size(list); i++) {
//        printf("%s\n", (char *) list_getItem(list, i));
//    }

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
