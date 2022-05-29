#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "esp_log.h"
#include "utils.h"

private uint64_t counter;

private void setup() {

}

private void loop() {
    counter++;
    if (counter % 1000) {
        ESP_LOGI("TAG", "Counter: %llu\n", counter);
    }
}

public void app_main() {
    setup();
    while (true) {
        loop();
    }
}
