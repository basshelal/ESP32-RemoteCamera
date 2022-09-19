#ifndef ESP32_REMOTECAMERA_BATTERY_H
#define ESP32_REMOTECAMERA_BATTERY_H

#include <stdbool.h>
#include "Error.h"
#include "Utils.h"

typedef struct BatteryInfo{
    bool isCharging;
    float voltage;
    float percentage;
} BatteryInfo;

typedef void(*PercentageChangedCallback)(const float oldValue, const float newValue);

typedef void(*IsChargingChangedCallback)(const bool oldIsCharging, const bool newIsCharging);

extern Error battery_init();

extern void battery_setSampleCount(const uint samplesCount);

extern uint battery_getSampleCount();

extern bool battery_isCharging();

extern float battery_getPercentage();

extern float battery_getVoltage();

extern Error battery_getInfo(BatteryInfo *batteryInfo);

extern void battery_addOnPercentageChangedCallback(PercentageChangedCallback percentageChangedCallback);

extern void battery_removeOnPercentageChangedCallback(PercentageChangedCallback percentageChangedCallback);

extern void battery_addIsChargingChangedCallback(IsChargingChangedCallback isChargingChangedCallback);

extern void battery_removeIsChargingChangedCallback(IsChargingChangedCallback isChargingChangedCallback);

#endif //ESP32_REMOTECAMERA_BATTERY_H
