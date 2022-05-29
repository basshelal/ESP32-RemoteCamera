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

#define WIFI_TAG "WiFi"

#define WIFI_CONNECT_RETRY_ATTEMPTS 10
private uint8_t retryCount = 0;

private esp_err_t err;

private void wifi_eventHandler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(WIFI_TAG, "Connecting to Access Point");
                err = esp_wifi_connect();
                if (err) ESP_LOGE(WIFI_TAG, "Connect returned error: %i : %s", err, esp_err_to_name(err));
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (retryCount < WIFI_CONNECT_RETRY_ATTEMPTS) {
                    esp_wifi_connect();
                    if (err) ESP_LOGE(WIFI_TAG, "Connect returned error: %i : %s", err, esp_err_to_name(err));
                    retryCount++;
                    ESP_LOGI(WIFI_TAG, "Retrying to connect to the Access Point, attempt: %i/%i",
                             retryCount, WIFI_CONNECT_RETRY_ATTEMPTS);
                } else {
                    xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
                }
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(WIFI_TAG, "Connected to Access Point");
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(WIFI_TAG, "IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

private void wifi_initSTA() {
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
                                                        &wifi_eventHandler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_eventHandler,
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

    ESP_LOGI(WIFI_TAG, "Finished WiFi initialization, waiting to connect...");

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
        ESP_LOGI(WIFI_TAG, "Connected to Access Point SSID:%s Password:%s",
                 WIFI_SSID, WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(WIFI_TAG, "Failed to connect to SSID:%s Password:%s",
                 WIFI_SSID, WIFI_PASSWORD);
    } else {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(wifiEventGroup);
}

public esp_err_t wifi_init() {
    // Initialize NVS which is needed for some reason
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(WIFI_TAG, "Initializing WiFi in STA mode");
    wifi_initSTA();
    return 0;
}