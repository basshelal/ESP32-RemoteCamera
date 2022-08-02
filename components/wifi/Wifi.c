#include "Wifi.h"
#include "Logger.h"
#include "secrets.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "StorageError.h"
#include "InternalStorage.h"

// From secrets.h a git-ignored file
#define WIFI_SSID SECRET_WIFI_SSID
#define WIFI_PASSWORD SECRET_WIFI_PASSWORD

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64

#define WIFI_CONNECT_RETRY_ATTEMPTS 10

#define requireInitialized(message, ...) \
require(this.initialized, WIFI_ERROR_WIFI_NOT_INITIALIZED, "Wifi not initialized! " message , ##__VA_ARGS__)

#define checkESPError(err, func, error, message, ...) \
require(err == ESP_OK, error, func"() returned %i : %s\n"message, err, esp_err_to_name(err), ##__VA_ARGS__)

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
                INFO("Connecting to Access Point");
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
                    INFO("Retrying to connect to the Access Point, attempt: %i/%i",
                         retryCount, WIFI_CONNECT_RETRY_ATTEMPTS);
                } else {
                    xEventGroupSetBits(this.eventGroupSTA, WIFI_FAIL_BIT);
                }
                break;
            case WIFI_EVENT_STA_CONNECTED:
                INFO("Connected to Access Point");
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        INFO("IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(this.eventGroupSTA, WIFI_CONNECTED_BIT);
    }
}

private inline esp_err_t wifi_ipAddressUInt32ToUInt8Array(const uint32_t uint32In,
                                                          uint8_t *arrayResult,
                                                          const size_t arrayLength) {
    requireNotNull(arrayResult, ESP_FAIL, "arrayResult must not be NULL");
    require(arrayLength >= 4, ESP_FAIL, "arrayLength must be >= 4");
    // Decode uint32 into uint8 array for each entry in the IP address (192.168.0.255) each is a uint8
    arrayResult[0] = (uint8_t) (uint32In >> (32 - (8 * 4)));
    arrayResult[1] = (uint8_t) (uint32In >> (32 - (8 * 3)));
    arrayResult[2] = (uint8_t) (uint32In >> (32 - (8 * 2)));
    arrayResult[3] = (uint8_t) (uint32In >> (32 - (8 * 1)));
    return ESP_OK;
}

private inline esp_err_t wifi_ipAddressUInt8ArrayToUInt32(uint32_t *uint32Result,
                                                          const uint8_t *arrayIn,
                                                          const size_t arrayLength) {
    requireNotNull(arrayIn, ESP_FAIL, "arrayIn must not be NULL");
    requireNotNull(uint32Result, ESP_FAIL, "uint32Result must not be NULL");
    require(arrayLength >= 4, ESP_FAIL, "arrayLength must be >= 4");
    *uint32Result = 0;
    *uint32Result |= (arrayIn[0] << (8 * 0));
    *uint32Result |= (arrayIn[1] << (8 * 1));
    *uint32Result |= (arrayIn[2] << (8 * 2));
    *uint32Result |= (arrayIn[3] << (8 * 3));
    return ESP_OK;
}

private bool wifi_hasWifiCredentialsInStorage() {
    return internalStorage_hasKey(INTERNAL_STORAGE_KEY_WIFI_SSID) &&
           internalStorage_hasKey(INTERNAL_STORAGE_KEY_WIFI_PASSWORD) &&
           internalStorage_hasKey(INTERNAL_STORAGE_KEY_WIFI_IP_ADDRESS);
}

private StorageError wifi_retrieveWifiCredentialsFromStorage(
        char *wifiSSID, char *wifiPassword, uint32_t *ipAddress) {
    requireNotNull(wifiSSID, STORAGE_ERROR_INVALID_PARAMETER, "wifiSSID cannot be NULL");
    requireNotNull(wifiPassword, STORAGE_ERROR_INVALID_PARAMETER, "wifiPassword cannot be NULL");
    requireNotNull(ipAddress, STORAGE_ERROR_INVALID_PARAMETER, "ipAddress cannot be NULL");

    StorageError err;
    err = internalStorage_getString(INTERNAL_STORAGE_KEY_WIFI_SSID, wifiSSID);
    err = internalStorage_getString(INTERNAL_STORAGE_KEY_WIFI_PASSWORD, wifiPassword);
    err = internalStorage_getUInt32(INTERNAL_STORAGE_KEY_WIFI_IP_ADDRESS, ipAddress);
    // TODO: 01-Aug-2022 @basshelal: Error checks!

    // TODO: 01-Aug-2022 @basshelal: Hardcoded values until we implement proper setting of these values
    strcpy(wifiSSID, WIFI_SSID);
    strcpy(wifiPassword, WIFI_PASSWORD);
    uint8_t ipArray[4] = {192, 168, 0, 123};
    wifi_ipAddressUInt8ArrayToUInt32(ipAddress, ipArray, sizeof(ipArray));

    return STORAGE_ERROR_NONE;
}

private WifiError wifi_connectSTA() {
    esp_err_t err;
    requireInitialized("Cannot start Wifi in STA mode");

    char wifiSSID[WIFI_SSID_MAX_LENGTH];
    char wifiPassword[WIFI_PASSWORD_MAX_LENGTH];
    uint32_t ipAddress;
    StorageError storageError = wifi_retrieveWifiCredentialsFromStorage(wifiSSID, wifiPassword, &ipAddress);

    this.eventGroupSTA = xEventGroupCreate();
    if (this.eventGroupSTA == NULL) {
        throw(WIFI_ERROR_GENERIC_FAILURE, "xEventGroupCreate() returned NULL, FreeRTOS has no more memory to allocate");
    }

    err = esp_event_loop_create_default();
    checkESPError(err, "esp_event_loop_create_default", WIFI_ERROR_GENERIC_FAILURE,);

    esp_netif_t *espNetIf = esp_netif_create_default_wifi_sta();
    esp_netif_dhcpc_stop(espNetIf); // ignore errors
    esp_netif_ip_info_t ipInfo;
    ipInfo.ip.addr = ipAddress;
    IP4_ADDR(&ipInfo.gw, 192, 168, 0, 1);
    IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
    err = esp_netif_set_ip_info(espNetIf, &ipInfo);
    if (err == ESP_ERR_ESP_NETIF_DHCP_NOT_STOPPED) { // try again!
        esp_netif_dhcpc_stop(espNetIf); // ignore errors
    }

    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&wifiInitConfig);
    checkESPError(err, "esp_wifi_init", WIFI_ERROR_GENERIC_FAILURE,);

    esp_event_handler_instance_t wifiEventHandlerInstance;
    esp_event_handler_instance_t gotIPEventHandlerInstance;
    err = esp_event_handler_instance_register(
            /*event_base=*/ WIFI_EVENT,
            /*event_id=*/ ESP_EVENT_ANY_ID,
            /*event_handler=*/ &wifi_eventHandlerSTAConnect,
            /*event_handler_arg=*/ NULL,
            /*instance=*/ &wifiEventHandlerInstance);
    checkESPError(err, "esp_event_handler_instance_register", WIFI_ERROR_GENERIC_FAILURE,);
    err = esp_event_handler_instance_register(
            /*event_base=*/ IP_EVENT,
            /*event_id=*/ IP_EVENT_STA_GOT_IP,
            /*event_handler=*/ &wifi_eventHandlerSTAConnect,
            /*event_handler_arg=*/ NULL,
            /*instance=*/ &gotIPEventHandlerInstance);
    checkESPError(err, "esp_event_handler_instance_register", WIFI_ERROR_GENERIC_FAILURE,);

    wifi_config_t wifiConfig = {.sta = {}};
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    strcpy((char *) wifiConfig.sta.ssid, wifiSSID);
    strcpy((char *) wifiConfig.sta.password, wifiPassword);

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    checkESPError(err, "esp_wifi_set_mode", WIFI_ERROR_GENERIC_FAILURE,);
    err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    checkESPError(err, "esp_wifi_set_config", WIFI_ERROR_GENERIC_FAILURE,);
    err = esp_wifi_start();
    checkESPError(err, "esp_wifi_start", WIFI_ERROR_GENERIC_FAILURE,);

    this.connectionState = CONNECTING;
    INFO("Finished Wifi initialization, connecting...");

    /* Wait (block) until either Wifi connected or Wifi failed */
    EventBits_t bits = xEventGroupWaitBits(
            /*xEventGroup=*/ this.eventGroupSTA,
            /*uxBitsToWaitFor=*/ WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            /*xClearOnExit=*/ pdFALSE,
            /*xWaitForAllBits=*/ pdFALSE,
            /*xTicksToWait=*/ portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        INFO("Connected to Access Point SSID:%s Password:%s", WIFI_SSID, WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        INFO("Failed to connect to SSID:%s Password:%s", WIFI_SSID, WIFI_PASSWORD);
    } else {
        throw(WIFI_ERROR_GENERIC_FAILURE, "xEventGroupWaitBits() returned unknown bits: %i", bits);
    }

    err = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, gotIPEventHandlerInstance);
    checkESPError(err, "esp_event_handler_instance_unregister", WIFI_ERROR_GENERIC_FAILURE,);
    err = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandlerInstance);
    checkESPError(err, "esp_event_handler_instance_unregister", WIFI_ERROR_GENERIC_FAILURE,);
    vEventGroupDelete(this.eventGroupSTA);
    this.eventGroupSTA = NULL;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_connectAP() {
    requireInitialized();
    this.connectionState = CONNECTING;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_disconnectSTA() {
    requireInitialized();
    this.connectionState = DISCONNECTED;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_disconnectAP() {
    requireInitialized();
    this.connectionState = DISCONNECTED;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_abortConnectSTA() {
    requireInitialized();
    this.connectionState = DISCONNECTING;
    return WIFI_ERROR_NONE;
}

private WifiError wifi_abortConnectAP() {
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
                    err = wifi_connectSTA();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        this.wifiMode = wifiMode;
                    }
                    break;
                case WIFI_MODE_AP:
                    err = wifi_connectAP();
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
                    err = wifi_abortConnectSTA();
                    if (err != WIFI_ERROR_NONE) {
                        // TODO: 30-Jul-2022 @basshelal:
                        //  "Error occurred trying to abort STA mode connection for reconnection";
                    } else {
                        return wifi_connect(wifiMode);
                    }
                    break;
                case WIFI_MODE_AP:
                    err = wifi_abortConnectAP();
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
                    err = wifi_disconnectSTA();
                    if (err != WIFI_ERROR_NONE) {

                    } else {
                        return wifi_connect(wifiMode);
                    }
                    break;
                case WIFI_MODE_AP:
                    err = wifi_disconnectAP();
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
            ERROR("Cannot start while already stopping, wait until successfully disconnected");
            return ESP_FAIL;
        default:
            ERROR("Cannot start wifi, unexpected current state: %i", this.connectionState);
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
                    wifi_abortConnectSTA();
                    break;
                case WIFI_MODE_AP:
                    wifi_abortConnectAP();
                    break;
                default:
                    break;
            }
            break;
        case CONNECTED: // Connected, disconnect fully as normal
            switch (this.wifiMode) {
                case WIFI_MODE_STA:
                    wifi_disconnectSTA();
                    break;
                case WIFI_MODE_AP:
                    wifi_disconnectAP();
                    break;
                default:
                    break;
            }
            break;
        case DISCONNECTING: // Already disconnecting, return with error info
            ERROR("Cannot stop while already stopping, wait until successfully disconnected");
            return ESP_FAIL;
        default:
            ERROR("Cannot stop wifi, unexpected current state: %i", this.connectionState);
            return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}