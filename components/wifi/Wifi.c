#include "Wifi.h"
#include "Logger.h"
#include "secrets.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "InternalStorage.h"

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

#define requireInitialized(format, ...) \
do { if(!this.initialized) {\
    throw(WIFI_ERROR_WIFI_NOT_INITIALIZED,"Wifi not initialized! " format , ##__VA_ARGS__);\
}} while(0)

private void thisReset();

private struct {
    bool initialized;
    WifiMode wifiMode;
    WifiConnectionState connectionState;
    /** FreeRTOS event group to signal when we are connected */
    EventGroupHandle_t eventGroupSTA;
    void (*const reset)();
} this = {
        .initialized = false,
        .wifiMode = WIFI_MODE_NULL,
        .connectionState = DISCONNECTED,
        .eventGroupSTA = NULL,
        .reset = thisReset
};

private void thisReset() {
    this.initialized = false;
    this.wifiMode = WIFI_MODE_NULL;
    this.connectionState = DISCONNECTED;
    if (this.eventGroupSTA != NULL) {
        vEventGroupDelete(this.eventGroupSTA);
    }
    this.eventGroupSTA = NULL;
}

private void wifi_eventHandlerSTAConnect(void *arg, esp_event_base_t event_base,
                                         int32_t event_id, void *event_data) {
    esp_err_t err;
    uint8_t retryCount = 0;
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                LOGI("Connecting to Access Point");
                err = esp_wifi_connect();
                if (err != ESP_OK) {
                    // TODO: 31-Jul-2022 @basshelal:
                    TODO("Implement!");
                }
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (retryCount < WIFI_CONNECT_RETRY_ATTEMPTS) {
                    err = esp_wifi_connect();
                    if (err != ESP_OK) {
                        // TODO: 31-Jul-2022 @basshelal:
                        TODO("Implement!");
                    }
                    retryCount++;
                    LOGI("Retrying to connect to the Access Point, attempt: %i/%i",
                         retryCount, WIFI_CONNECT_RETRY_ATTEMPTS);
                } else {
                    xEventGroupSetBits(this.eventGroupSTA, WIFI_FAIL_BIT);
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
        xEventGroupSetBits(this.eventGroupSTA, WIFI_CONNECTED_BIT);
    }
}

private esp_err_t wifi_startSTA() {
    esp_err_t err;
    requireInitialized("Cannot start Wifi in STA mode");

    StorageError storageError;
    char wifiSSID[32];
    char wifiPassword[64];
    uint32_t ipAddress;
    storageError = internalStorage_getString(INTERNAL_STORAGE_KEY_WIFI_SSID, wifiSSID);
    storageError = internalStorage_getString(INTERNAL_STORAGE_KEY_WIFI_PASSWORD, wifiPassword);
    storageError = internalStorage_getUInt32(INTERNAL_STORAGE_KEY_WIFI_IP_ADDRESS, &ipAddress);
    // Decode uint32 into uint8 array for each entry in the IP address (192.168.0.255) each is a uint8
    uint8_t ipAddressArray[4];
    ipAddressArray[0] = (uint8_t) (ipAddress >> (32 - (8 * 1)));
    ipAddressArray[1] = (uint8_t) (ipAddress >> (32 - (8 * 2)));
    ipAddressArray[2] = (uint8_t) (ipAddress >> (32 - (8 * 3)));
    ipAddressArray[3] = (uint8_t) (ipAddress >> (32 - (8 * 4)));
    strcpy(wifiSSID, WIFI_SSID);
    strcpy(wifiPassword, WIFI_PASSWORD);

    this.eventGroupSTA = xEventGroupCreate();
    if (this.eventGroupSTA == NULL) {
        throw(WIFI_ERROR_GENERIC_FAILURE, "xEventGroupCreate() returned NULL, FreeRTOS has no more memory to allocate");
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        throw(WIFI_ERROR_GENERIC_FAILURE, "esp_event_loop_create_default() returned %i : %s", err,
              esp_err_to_name(err));
    }

    // TODO: 30-Jul-2022 @basshelal: Clean this up and continue here
    esp_netif_t *espNetIf = esp_netif_create_default_wifi_sta();
    esp_netif_dhcpc_stop(espNetIf); // ignore errors
    esp_netif_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192, 168, 0, 123); // device's IP address is 192.168.0.123
    IP4_ADDR(&ipInfo.gw, 192, 168, 0, 1);
    IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
    err = esp_netif_set_ip_info(espNetIf, &ipInfo);
    if (err == ESP_ERR_ESP_NETIF_DHCP_NOT_STOPPED) { // try again!
        esp_netif_dhcpc_stop(espNetIf); // ignore errors
    }

    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&wifiInitConfig);
    if (err != ESP_OK) {
        throw(WIFI_ERROR_GENERIC_FAILURE, "esp_wifi_init() returned %i : %s", err, esp_err_to_name(err));
    }

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

    wifi_config_t wifiConfig = {
            .sta = {
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    strcpy((char *) wifiConfig.sta.ssid, wifiSSID);
    strcpy((char *) wifiConfig.sta.password, wifiPassword);

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    err = esp_wifi_start();

    this.connectionState = CONNECTING;
    LOGI("Finished Wifi initialization, waiting to connect...");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(this.eventGroupSTA,
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
    vEventGroupDelete(this.eventGroupSTA);
    this.eventGroupSTA = NULL;
    return ESP_OK;
}

private WifiError wifi_startAP() {
    requireInitialized();
    this.connectionState = CONNECTING;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_stopSTA() {
    requireInitialized();
    this.connectionState = DISCONNECTED;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_stopAP() {
    requireInitialized();
    this.connectionState = DISCONNECTED;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_abortSTA() {
    requireInitialized();
    this.connectionState = DISCONNECTING;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_abortAP() {
    requireInitialized();
    this.connectionState = DISCONNECTING;
    return WIFI_ERROR_NONE;
}

public WifiError wifi_init() {
    if (!this.initialized) {
        VERBOSE("Initializing Wifi");
        esp_err_t err;
        // Initialize internal storage for PHY calibration stored in NVS and for later fetching of Wifi details
        StorageError storageError = internalStorage_init();
        if (storageError != STORAGE_ERROR_NONE) {
            throw(WIFI_ERROR_GENERIC_FAILURE, "Could not initialize Wifi, internal storage initialization failed");
        }

        err = esp_netif_init();
        if (err != ESP_OK) {
            throw(WIFI_ERROR_GENERIC_FAILURE, "Could not initialize Wifi, esp_netif_init() returned %i: %s",
                  err, esp_err_to_name(err));
        }

        this.wifiMode = WIFI_MODE_NULL;
        this.connectionState = DISCONNECTED;
        this.initialized = true;
        VERBOSE("Successfully initialized Wifi");
    } else {
        WARN("Wifi has already initialized!");
    }
    return WIFI_ERROR_NONE;
}

public WifiError wifi_destroy() {
    requireInitialized();
    esp_netif_deinit();
    this.reset();
    return WIFI_ERROR_NONE;
}

public esp_err_t wifi_connect(const WifiMode wifiMode) {
    requireInitialized();
    esp_err_t err;
    switch (this.connectionState) {
        case DISCONNECTED: // Disconnected, start as normal
            switch (wifiMode) {
                case WIFI_MODE_STA:
                    err = wifi_startSTA();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        this.wifiMode = wifiMode;
                    }
                    break;
                case WIFI_MODE_AP:
                    err = wifi_startAP();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        this.wifiMode = wifiMode;
                    }
                    break;
                default:
                throw(WIFI_ERROR_INVALID_WIFI_MODE,
                      "Cannot connect Wifi to mode: %i, only STA (%i) and AP (%i) are allowed",
                      wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
            }
            this.connectionState = CONNECTED;
            break;
        case CONNECTING: // Already begun a connection attempt, abort and re-connect
            switch (this.wifiMode) {
                case WIFI_MODE_STA:
                    err = wifi_abortSTA();
                    if (err != WIFI_ERROR_NONE) {
                        // TODO: 30-Jul-2022 @basshelal:
                        //  "Error occurred trying to abort STA mode connection for reconnection";
                    } else {
                        return wifi_connect(wifiMode);
                    }
                    break;
                case WIFI_MODE_AP:
                    err = wifi_abortAP();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        return wifi_connect(wifiMode);
                    }
                    break;
                default:
                throw(WIFI_ERROR_INVALID_WIFI_MODE,
                      "Cannot connect Wifi to mode: %i, only STA (%i) and AP (%i) are allowed",
                      wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
            }
            break;
        case CONNECTED: // Already connected, disconnect and re-connect
            switch (this.wifiMode) {
                case WIFI_MODE_STA:
                    err = wifi_stopSTA();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        return wifi_connect(wifiMode);
                    }
                    break;
                case WIFI_MODE_AP:
                    err = wifi_stopAP();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        return wifi_connect(wifiMode);
                    }
                    break;
                default:
                throw(WIFI_ERROR_INVALID_WIFI_MODE,
                      "Cannot connect Wifi to mode: %i, only STA (%i) and AP (%i) are allowed",
                      wifiMode, WIFI_MODE_STA, WIFI_MODE_AP);
            }
            break;
        case DISCONNECTING: // Disconnecting, return error with info, cannot do anything until successful disconnection
            LOGE("Cannot start while already stopping, wait until successfully disconnected");
            return ESP_FAIL;
        default:
            LOGE("Cannot start wifi, unexpected current state: %i", this.connectionState);
            return ESP_ERR_INVALID_STATE;
    }
    return WIFI_ERROR_NONE;
}

public WifiMode wifi_getMode() {
    return this.wifiMode;
}

public WifiConnectionState wifi_getConnectionState() {
    return this.connectionState;
}

public esp_err_t wifi_disconnect() {
    requireInitialized();
    switch (this.connectionState) {
        case DISCONNECTED: // Already disconnected, do nothing
            break;
        case CONNECTING: // Attempting to connect, abort connection
            switch (this.wifiMode) {
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
            switch (this.wifiMode) {
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
            LOGE("Cannot stop wifi, unexpected current state: %i", this.connectionState);
            return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}