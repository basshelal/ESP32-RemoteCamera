#include "utils.h"
#include "services/wifi.h"

private void setup() {
    wifi_init();
}

private void loop() {

}

_Noreturn
public  void app_main() {
    setup();
    while (true) {
        loop();
    }
}
