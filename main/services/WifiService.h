#ifndef ESP32_REMOTECAMERA_WIFISERVICE_H
#define ESP32_REMOTECAMERA_WIFISERVICE_H

#include <esp_err.h>
#include <esp_wifi_types.h>

/** Alias for ESP-IDF's Wifi Modes, however, only  WIFI_MODE_STA and WIFI_MODE_AP are supported */
typedef wifi_mode_t WifiMode;

/** Enum for all possible connection states */
typedef enum {
    /** The initial state after calling init, goes only to CONNECTING after a call to start */
    DISCONNECTED,
    /** Intermediate state after calling start, goes to CONNECTED upon a successful connection or
     * DISCONNECTING if stop is called before a successful connection */
    CONNECTING,
    /** Successfully connected, goes only to DISCONNECTING after a call to stop */
    CONNECTED,
    /** Intermediate state after calling stop, goes only to DISCONNECTED */
    DISCONNECTING,
} WifiConnectionState;

extern struct {
    esp_err_t (*init)();
    esp_err_t (*connect)(const WifiMode wifiMode);
    WifiMode (*getCurrentMode)();
    WifiConnectionState (*getCurrentConnectionState)();
    esp_err_t (*disconnect)();
} WifiService;

extern esp_err_t wifi_init();

extern esp_err_t wifi_connect(const WifiMode wifiMode);

extern WifiMode wifi_getCurrentMode();

extern WifiConnectionState wifi_getCurrentConnectionState();

extern esp_err_t wifi_disconnect();

#endif //ESP32_REMOTECAMERA_WIFISERVICE_H
