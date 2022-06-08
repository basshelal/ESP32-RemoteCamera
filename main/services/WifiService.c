#include "WifiService.h"
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

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_CONNECT_RETRY_ATTEMPTS 10
#define WIFI_TAG "Wifi Service"
#define LOGI(format, ...) ESP_LOGI(WIFI_TAG, format, ##__VA_ARGS__)
#define LOGE(format, ...) ESP_LOGE(WIFI_TAG, format, ##__VA_ARGS__)

/** Check, log and return error, all in one macro call */
#define CHECK_ERROR(_err, _operation, _format, ...)                        \
({if((_err = _operation)) {                                                \
    LOGE(_format " E: %i: %s", ##__VA_ARGS__, _err, esp_err_to_name(_err));\
}_err;})

private bool initCalled = false;

#define CHECK_INIT(format, ...)                    \
if(!initCalled) {                                  \
    LOGE(format " init not called", ##__VA_ARGS__);\
    return ESP_ERR_INVALID_STATE;                  \
}

private WifiMode currentWifiMode = WIFI_MODE_NULL;
private WifiConnectionState currentState = DISCONNECTED;
/** FreeRTOS event group to signal when we are connected */
private EventGroupHandle_t eventGroupSTA;

private void wifi_eventHandlerSTAConnect(void *arg, esp_event_base_t event_base,
                                         int32_t event_id, void *event_data) {
    esp_err_t err;
    uint8_t retryCount = 0;
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                LOGI("Connecting to Access Point");
                CHECK_ERROR(err, esp_wifi_connect(), "Failed to connect");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (retryCount < WIFI_CONNECT_RETRY_ATTEMPTS) {
                    CHECK_ERROR(err, esp_wifi_connect(), "Failed to connect");
                    retryCount++;
                    LOGI("Retrying to connect to the Access Point, attempt: %i/%i",
                         retryCount, WIFI_CONNECT_RETRY_ATTEMPTS);
                } else {
                    xEventGroupSetBits(eventGroupSTA, WIFI_FAIL_BIT);
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
        xEventGroupSetBits(eventGroupSTA, WIFI_CONNECTED_BIT);
    }
}

private esp_err_t wifi_startSTA() {
    CHECK_INIT("Cannot start Wifi");
    eventGroupSTA = xEventGroupCreate();

    // TODO: 06-Jun-2022 @basshelal: Don't use default event loop in case it's going to be used by something else
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

    currentState = CONNECTING;
    LOGI("Finished Wifi initialization, waiting to connect...");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(eventGroupSTA,
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
    vEventGroupDelete(eventGroupSTA);
    return ESP_OK;
}

private esp_err_t wifi_startAP() {
    CHECK_INIT();
    currentState = CONNECTING;
    return ESP_OK;
}

private esp_err_t wifi_stopSTA() {
    CHECK_INIT();
    currentState = DISCONNECTED;
    return ESP_OK;
}

private esp_err_t wifi_stopAP() {
    CHECK_INIT();
    currentState = DISCONNECTED;
    return ESP_OK;
}

private esp_err_t wifi_abortSTA() {
    CHECK_INIT();
    currentState = DISCONNECTING;
    return ESP_OK;
}

private esp_err_t wifi_abortAP() {
    CHECK_INIT();
    currentState = DISCONNECTING;
    return ESP_OK;
}

public esp_err_t wifi_init() {
    esp_err_t err;
    if (initCalled) {
        LOGI("Init was already called!");
        return ESP_OK;
    }
    // Initialize NVS which is needed for WiFi and Bluetooth, see https://community.platformio.org/t/understanding-nvs-better/25548
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (CHECK_ERROR(err, nvs_flash_erase(), "Could not erase flash for NVS init"))
            return err;
    }
    if (CHECK_ERROR(err, nvs_flash_init(), "Could not initialize NVS"))
        return err;

    if (CHECK_ERROR(err, esp_netif_init(), "Could not initialize NETIF"))
        return err;

    currentWifiMode = WIFI_MODE_NULL;
    currentState = DISCONNECTED;
    initCalled = true;
    return ESP_OK;
}

public esp_err_t wifi_connect(const WifiMode wifiMode) { // NOLINT(misc-no-recursion)
    CHECK_INIT();
    esp_err_t err;
    switch (currentState) {
        case DISCONNECTED: // Disconnected, start as normal
            switch (wifiMode) {
                case WIFI_MODE_STA:
                    currentWifiMode = wifiMode;
                    LOGI("Initializing Wifi service in STA mode");
                    if (CHECK_ERROR(err, wifi_startSTA(), "Error occurred trying to start in STA mode"))
                        return err;
                    break;
                case WIFI_MODE_AP:
                    currentWifiMode = wifiMode;
                    LOGI("Initializing Wifi service in AP mode");
                    if (CHECK_ERROR(err, wifi_startAP(), "Error occurred trying to start in AP mode"))
                        return err;
                    break;
                default:
                    LOGE("Cannot start Wifi service in mode: %i, only STA (%i) and AP (%i) are allowed",
                         wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
                    return ESP_ERR_INVALID_ARG;
            }
            currentState = CONNECTED;
            return ESP_OK;
        case CONNECTING: // Already begun a connection attempt, abort and re-connect
            switch (currentWifiMode) {
                case WIFI_MODE_STA:
                    if (CHECK_ERROR(err, wifi_abortSTA(),
                                    "Error occurred trying to abort STA mode connection for reconnection"))
                        return err;
                    wifi_connect(wifiMode);
                    break;
                case WIFI_MODE_AP:
                    wifi_abortAP();
                    wifi_connect(wifiMode);
                    break;
                default:
                    LOGE("Cannot start Wifi service in mode: %i, only STA (%i) and AP (%i) are allowed",
                         wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
                    return ESP_ERR_INVALID_ARG;
            }
            break;
        case CONNECTED: // Already connected, disconnect and re-connect
            switch (currentWifiMode) {
                case WIFI_MODE_STA:
                    wifi_stopSTA();
                    wifi_connect(wifiMode);
                    break;
                case WIFI_MODE_AP:
                    wifi_stopAP();
                    wifi_connect(wifiMode);
                    break;
                default:
                    LOGE("Cannot start Wifi service in mode: %i, only STA (%i) and AP (%i) are allowed",
                         wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
                    return ESP_ERR_INVALID_ARG;
            }
            break;
        case DISCONNECTING: // Disconnecting, return error with info, cannot do anything until successful disconnection
            LOGE("Cannot start while already stopping, wait until successfully disconnected");
            return ESP_FAIL;
        default:
            LOGE("Cannot start wifi, unexpected current state: %i", currentState);
            return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

public WifiMode wifi_getCurrentMode() { return currentWifiMode; }

public WifiConnectionState wifi_getCurrentConnectionState() { return currentState; }

public esp_err_t wifi_disconnect() {
    CHECK_INIT();
    switch (currentState) {
        case DISCONNECTED: // Already disconnected, do nothing
            break;
        case CONNECTING: // Attempting to connect, abort connection
            switch (currentWifiMode) {
                case WIFI_MODE_STA:
                    wifi_abortSTA();
                    break;
                case WIFI_MODE_AP:
                    wifi_abortAP();
                    break;
                default:
                    break;
            }
            break;
        case CONNECTED: // Connected, disconnect fully as normal
            switch (currentWifiMode) {
                case WIFI_MODE_STA:
                    wifi_stopSTA();
                    break;
                case WIFI_MODE_AP:
                    wifi_stopAP();
                    break;
                default:
                    break;
            }
            break;
        case DISCONNECTING: // Already disconnecting, return with error info
            LOGE("Cannot stop while already stopping, wait until successfully disconnected");
            return ESP_FAIL;
        default:
            LOGE("Cannot stop wifi, unexpected current state: %i", currentState);
            return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

public typeof(WifiService) WifiService = {
        .init = wifi_init,
        .connect = wifi_connect,
        .getCurrentMode = wifi_getCurrentMode,
        .getCurrentConnectionState = wifi_getCurrentConnectionState,
        .disconnect = wifi_disconnect,
};