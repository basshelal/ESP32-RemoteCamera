#include "wifi.h"
#include "utils.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "secrets.h"

// From secrets.h a git-ignored file
#define WIFI_SSID SECRET_WIFI_SSID
#define WIFI_PASSWORD SECRET_WIFI_PASSWORD

/** FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t wifiEventGroup;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_TAG "Wifi Service"
#define WIFI_CONNECT_RETRY_ATTEMPTS 10
#define LOGI(format, ...) ESP_LOGI(WIFI_TAG, format, ##__VA_ARGS__)
#define LOGE(format, ...) ESP_LOGE(WIFI_TAG, format, ##__VA_ARGS__)

private WifiMode currentWifiMode = WIFI_MODE_NULL;
private enum {
    STOPPED, STARTED
} currentState;

private void wifi_eventHandlerSTAConnect(void *arg, esp_event_base_t event_base,
                                         int32_t event_id, void *event_data) {
    esp_err_t err = ESP_OK;
    uint8_t retryCount = 0;
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                LOGI("Connecting to Access Point");
                if ((err = esp_wifi_connect())) {
                    LOGE("Connect returned error: %i : %s", err, esp_err_to_name(err));
                }
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (retryCount < WIFI_CONNECT_RETRY_ATTEMPTS) {
                    if ((err = esp_wifi_connect())) {
                        LOGE("Connect returned error: %i : %s", err, esp_err_to_name(err));
                    }
                    retryCount++;
                    LOGI("Retrying to connect to the Access Point, attempt: %i/%i",
                         retryCount, WIFI_CONNECT_RETRY_ATTEMPTS);
                } else {
                    xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
                }
                break;
            case WIFI_EVENT_STA_CONNECTED:
                LOGI("Connected to Access Point");
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        LOGI("IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

private void wifi_startSTA() {
    wifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_eventHandlerSTAConnect,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_eventHandlerSTAConnect,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = WIFI_SSID,
                    .password = WIFI_PASSWORD,
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    LOGI("Finished Wifi initialization, waiting to connect...");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        LOGI("Connected to Access Point SSID:%s Password:%s",
             WIFI_SSID, WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        LOGI("Failed to connect to SSID:%s Password:%s",
             WIFI_SSID, WIFI_PASSWORD);
    } else {
        LOGE("UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(wifiEventGroup);
}

private void wifi_startAP() {

}

public esp_err_t wifi_init() {
    // Initialize NVS which is needed for WiFi and Bluetooth, see https://community.platformio.org/t/understanding-nvs-better/25548
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    currentWifiMode = WIFI_MODE_NULL;
    currentState = STOPPED;
    return ESP_OK;
}

public esp_err_t wifi_start(WifiMode wifiMode) {
    if (wifiMode == WIFI_MODE_STA) {
        currentWifiMode = wifiMode;
        LOGI("Initializing Wifi service in STA mode");
        wifi_startSTA();
    } else if (wifiMode == WIFI_MODE_AP) {
        currentWifiMode = wifiMode;
        LOGI("Initializing Wifi service in AP mode");
        wifi_startAP();
    } else {
        LOGE("Cannot start Wifi service in mode: %i, only STA (%i) and AP (%i) are allowed",
             wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
        return ESP_ERR_INVALID_ARG;
    }
    currentState = STARTED;
    return ESP_OK;
}

public WifiMode wifi_getCurrentMode() {
    return currentWifiMode;
}

public esp_err_t wifi_stop() {
    return ESP_OK;
}