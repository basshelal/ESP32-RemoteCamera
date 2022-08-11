#include "Battery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "esp_adc_cal.h"
#include "Logger.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// TODO: 17-Jul-2022 @basshelal: For more realtime accurate battery readings as well as determining if we are
//  plugged in and charging the battery see: https://learn.adafruit.com/adafruit-huzzah32-esp32-feather?view=all#power-pins-2816344
//  If we connect and read some of these pins, namely BAT and USB we can determine the current power state
//  the ADC battery reading will be slightly higher when charging and so doesn't correctly represent the
//  battery's voltage, the BAT pin is the most accurate one

// TODO: 17-Jul-2022 @basshelal: Find a nice linear battery percentage function for this 3.7V battery because
//  it doesn't change voltage linearly it hangs at 3.7V most of its life peaking at ~4.2V and dying at ~3.2V

// TODO: 18-Jul-2022 @basshelal: Charging seems to add an additional .05V (50mV) to the battery

#define BATTERY_TASK_NAME "batteryTask"
#define BATTERY_TASK_STACK_SIZE 2200
#define BATTERY_TASK_PRIORITY tskIDLE_PRIORITY

private struct {
    adc1_channel_t batteryChannel;
    esp_adc_cal_characteristics_t *batteryCharacteristics;
    uint batterySamplesCount;
    // how many milliVolts determines if something changed ie margin of error
    float voltageReadingMarginOfError;
    // this many readings with a delta greater than margin of error means something truly changed
    uint voltageSampleMarginOfError;
    List *percentageChangedCallbacks;
    List *isChargingChangedCallbacks;
    struct {
        TaskHandle_t handle;
        bool isRunning;
        uint pollMillis;
        uint voltageSampleChangedCount; // to check against voltageSampleMarginOfError
        BatteryInfo previousBatteryInfo;
        BatteryInfo currentBatteryInfo;
    } task;
} this;

private uint battery_getRawReading(const uint samplesCount) {
    uint averagedReading = 0;
    uint samplesRetrieved = 0;
    for (uint i = 0; i < samplesCount; i++) {
        const int reading = adc1_get_raw(this.batteryChannel);
        if (reading >= 0) {
            averagedReading += reading;
            samplesRetrieved++;
        }
    }
    averagedReading /= samplesRetrieved;
    return averagedReading;
}

private float battery_getVoltageFromReading(const uint rawReading) {
    const uint voltage = esp_adc_cal_raw_to_voltage(rawReading, this.batteryCharacteristics);
    const float offset = battery_isCharging() ? 100.0F : 50.0F;
    // TODO: 11-Aug-2022 @basshelal: For some reason there is always a 50mV offset in the reading, not sure why
    const float result = (((float) voltage) * 2.0F) - offset;
    return result;
}

private float battery_getPercentageFromReading(const uint rawReading) {
    const float voltage = battery_getVoltageFromReading(rawReading);
    const float minOffset = 3300.0F;
    const float max = 4200.0F - minOffset; // 900 mV is max meaning range is from 0 - 900mV
    float offsetVoltage = voltage - minOffset;
    if (offsetVoltage < 0) {
        offsetVoltage = 0;
    } else if (offsetVoltage > max) {
        offsetVoltage = max;
    }
    const float percentage = (offsetVoltage / max) * 100.0F;
    return percentage;
}

private void battery_taskFunction(void *arg) {
    typeof(this) *thisPtr = (typeof(this) *) arg;
    while (thisPtr->task.isRunning) {
        const uint remaining = uxTaskGetStackHighWaterMark(thisPtr->task.handle);
        if (remaining < 128) { // quit task if we run out of stack to avoid program crash
            ERROR("Battery task ran out of stack, bytes remaining: %u", remaining);
            // TODO: 11-Aug-2022 @basshelal: Implement some safe restart mechanism because this needs to always be
            //  running
            break;
        }
        battery_getInfo(&thisPtr->task.currentBatteryInfo);
        const float previousVoltage = thisPtr->task.previousBatteryInfo.voltage;
        const float currentVoltage = thisPtr->task.currentBatteryInfo.voltage;
        const float previousPercentage = thisPtr->task.previousBatteryInfo.percentage;
        const float currentPercentage = thisPtr->task.currentBatteryInfo.percentage;
        const float voltageDifference = (float) fabs((double) previousVoltage - currentVoltage);
        if (voltageDifference > thisPtr->voltageReadingMarginOfError) {
            thisPtr->task.voltageSampleChangedCount++;
            if (thisPtr->task.voltageSampleChangedCount > thisPtr->voltageSampleMarginOfError) {
                const float percentageDifferance = (float) fabs((double) previousPercentage - currentPercentage);
                if (percentageDifferance > 0.0F) {
                    for (int i = 0; i < list_getSize(thisPtr->percentageChangedCallbacks); i++) {
                        const PercentageChangedCallback callback = list_getItem(thisPtr->percentageChangedCallbacks, i);
                        if (callback != NULL) {
                            callback(previousPercentage, currentPercentage);
                        }
                    }
                }
                thisPtr->task.voltageSampleChangedCount = 0;
                thisPtr->task.previousBatteryInfo = thisPtr->task.currentBatteryInfo;
            }
        }
        INFO("(%.f%%, %.fmV) -> (%.f%%, %.fmV)",
             previousPercentage, previousVoltage,
             currentPercentage, currentVoltage
        );
        vTaskDelay(pdMS_TO_TICKS(thisPtr->task.pollMillis));
    }
    vTaskDelete(thisPtr->task.handle);
}

private void battery_percentageChangedCallback(const float oldValue, const float newValue) {
    INFO("Percentage changed %f -> %f", oldValue, newValue);
}

public BatteryError battery_init() {
    this.batteryCharacteristics = new(esp_adc_cal_characteristics_t);
    this.batteryChannel = ADC1_CHANNEL_7;
    this.batterySamplesCount = 128;
    this.voltageReadingMarginOfError = 25.0F;
    this.voltageSampleMarginOfError = 3;
    this.percentageChangedCallbacks = list_create();
    this.isChargingChangedCallbacks = list_create();
    this.task.isRunning = true;
    this.task.pollMillis = 1000;

    list_addItem(this.percentageChangedCallbacks, battery_percentageChangedCallback);

    xTaskCreate(
            /*pvTaskCode=*/battery_taskFunction,
            /*pcName=*/BATTERY_TASK_NAME,
            /*usStackDepth=*/BATTERY_TASK_STACK_SIZE,
            /*pvParameters=*/&this,
            /*uxPriority=*/BATTERY_TASK_PRIORITY,
            /*pxCreatedTask=*/this.task.handle
    );

    const adc_bits_width_t batteryBitsWidth = ADC_WIDTH_BIT_12;
    const adc_atten_t batteryAttenuation = ADC_ATTEN_DB_11;
    const uint32_t defaultVRef = 1080;

    // Check and warn if we have no calibration values built-in
    esp_err_t hasVRef = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK;
    esp_err_t hasTwoPoint = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK;
    if (!hasVRef && !hasTwoPoint) {
        WARN("ADC does not have either of VRef or TwoPoint calibrations in eFuse, "
             "will use default (likely inaccurate value of %u", defaultVRef);
    }

    adc1_config_width(batteryBitsWidth);
    adc1_config_channel_atten(this.batteryChannel, batteryAttenuation);
    esp_adc_cal_value_t calibrationUsed = esp_adc_cal_characterize(ADC_UNIT_1, batteryAttenuation, batteryBitsWidth,
                                                                   defaultVRef, this.batteryCharacteristics);
    const char *calibrationToString;
    switch (calibrationUsed) {
        case ESP_ADC_CAL_VAL_EFUSE_VREF:
            calibrationToString = "eFuse VRef";
            break;
        case ESP_ADC_CAL_VAL_EFUSE_TP:
            calibrationToString = "eFuse TwoPoint";
            break;
        case ESP_ADC_CAL_VAL_EFUSE_TP_FIT:
            calibrationToString = "eFuse TwoPoint + Fitting Curve";
            break;
        case ESP_ADC_CAL_VAL_DEFAULT_VREF:
            calibrationToString = "Default VRef";
            break;
        default:
            calibrationToString = "unknown/other";
            break;
    }
    INFO("ADC Calibration used is: %s ", calibrationToString);

    return BATTERY_ERROR_NONE;
}

public void battery_setSampleCount(const uint samplesCount) { this.batterySamplesCount = samplesCount; }

public uint battery_getSampleCount() { return this.batterySamplesCount; }

public float battery_getVoltage() {
    return battery_getVoltageFromReading(battery_getRawReading(this.batterySamplesCount));
}

public float battery_getPercentage() {
    return battery_getPercentageFromReading(battery_getRawReading(this.batterySamplesCount));
}

public bool battery_isCharging() {
    // TODO: 10-Aug-2022 @basshelal: Implement!
    return false;
}

public BatteryError battery_getInfo(BatteryInfo *batteryInfo) {
    requireNotNull(batteryInfo, BATTERY_ERROR_INVALID_PARAMETER, "batterInfo cannot be null");
    const uint reading = battery_getRawReading(this.batterySamplesCount);
    const float voltage = battery_getVoltageFromReading(reading);
    const float percentage = battery_getPercentageFromReading(reading);
    const bool isCharging = battery_isCharging();
    batteryInfo->isCharging = isCharging;
    batteryInfo->voltage = voltage;
    batteryInfo->percentage = percentage;
    return BATTERY_ERROR_NONE;
}

public void battery_addOnPercentageChangedCallback(PercentageChangedCallback percentageChangedCallback) {
    list_addItem(this.percentageChangedCallbacks, percentageChangedCallback);
}

public void battery_removeOnPercentageChangedCallback(PercentageChangedCallback percentageChangedCallback) {
    list_removeItem(this.percentageChangedCallbacks, percentageChangedCallback);
}

public void battery_addIsChargingChangedCallback(IsChargingChangedCallback isChargingChangedCallback) {
    list_addItem(this.isChargingChangedCallbacks, isChargingChangedCallback);
}

public void battery_removeIsChargingChangedCallback(IsChargingChangedCallback isChargingChangedCallback) {
    list_removeItem(this.isChargingChangedCallbacks, isChargingChangedCallback);
}
