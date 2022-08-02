#ifndef ESP32_REMOTECAMERA_WIFI_H
#define ESP32_REMOTECAMERA_WIFI_H

#include "WifiError.h"
#include "Utils.h"
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

extern WifiError wifi_init();

extern WifiError wifi_destroy();

extern WifiError wifi_connect(const WifiMode wifiMode);

extern WifiMode wifi_getMode();

extern WifiConnectionState wifi_getConnectionState();

extern esp_err_t wifi_disconnect();

#endif //ESP32_REMOTECAMERA_WIFI_H
