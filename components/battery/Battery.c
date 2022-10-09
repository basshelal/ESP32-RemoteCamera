#include "Battery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "esp_adc_cal.h"
#include "Logger.h"
#include "TaskWatcher.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BATTERY_TASK_NAME "batteryTask"
#define BATTERY_TASK_STACK_SIZE 2200
#define BATTERY_TASK_STACK_MIN BATTERY_TASK_STACK_SIZE * 0.05
#define BATTERY_TASK_PRIORITY tskIDLE_PRIORITY
#define BATTERY_TASK_POLL_MILLIS 2000
#define BATTERY_MIN_VOLTAGE 2750.0F
#define BATTERY_MAX_VOLTAGE 4200.0F

private struct {
    adc1_channel_t batteryChannel;
    adc1_channel_t usbChannel;
    esp_adc_cal_characteristics_t *characteristics;
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
        uint voltageSampleChangedCount; // to check against voltageSampleMarginOfError
        BatteryInfo previousBatteryInfo;
        BatteryInfo currentBatteryInfo;
    } task;
} this;

private uint battery_getAveragedReading(const adc1_channel_t channel, const uint samplesCount) {
    uint averagedReading = 0;
    uint samplesRetrieved = 0;
    for (uint i = 0; i < samplesCount; i++) {
        const int reading = adc1_get_raw(channel);
        if (reading >= 0) {
            averagedReading += reading;
            samplesRetrieved++;
        }
    }
    averagedReading /= samplesRetrieved;
    return averagedReading;
}

private uint battery_getUsbRawReading(const uint samplesCount) {
    return battery_getAveragedReading(this.usbChannel, samplesCount);
}

private uint battery_getVoltageRawReading(const uint samplesCount) {
    return battery_getAveragedReading(this.batteryChannel, samplesCount);
}

private float battery_getIsChargingVoltage(const uint rawReading) {
    const uint voltage = esp_adc_cal_raw_to_voltage(rawReading, this.characteristics);
    const float result = (float) voltage;
    return result;
}

private float battery_getVoltageFromReading(const uint rawReading, const bool isCharging) {
    const uint voltage = esp_adc_cal_raw_to_voltage(rawReading, this.characteristics);
    const float offset = /*isCharging ? 100.0F :*/ 50.0F;
    // above is just a fact from experience, can't explain why, probably the regulator or something
    const float result = (((float) voltage) * 2.0F) - offset;
    return result;
}

private float battery_getPercentageFromReading(const uint rawReading, const bool isCharging) {
    const float voltage = battery_getVoltageFromReading(rawReading, isCharging);
    const float max = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;
    float offsetVoltage = voltage - BATTERY_MIN_VOLTAGE;
    if (offsetVoltage < 0) {
        offsetVoltage = 0;
    } else if (offsetVoltage > max) {
        offsetVoltage = max;
    }
    const float percentage = (offsetVoltage / max) * 100.0F;
    return percentage;
}

private void battery_taskFunction(void *arg);

private void battery_startBatteryTask() {
    this.task.isRunning = true;
    xTaskCreate(
            /*pvTaskCode=*/battery_taskFunction,
            /*pcName=*/BATTERY_TASK_NAME,
            /*usStackDepth=*/BATTERY_TASK_STACK_SIZE,
            /*pvParameters=*/&this,
            /*uxPriority=*/BATTERY_TASK_PRIORITY,
            /*pxCreatedTask=*/this.task.handle
    );
}

private void battery_taskFunction(void *arg) {
    taskWatcher_notifyTaskStarted(BATTERY_TASK_NAME);
    typeof(this) *thisPtr = (typeof(this) *) arg;
    // initialize previous info to avoid a change event on first run because previous was all 0s
    battery_getInfo(&thisPtr->task.previousBatteryInfo);
    uint remaining = 0;
    while (thisPtr->task.isRunning) {
        remaining = uxTaskGetStackHighWaterMark(thisPtr->task.handle);
        if (remaining < BATTERY_TASK_STACK_MIN) { // quit task if we run out of stack to avoid program crash
            ERROR("Battery task ran out of stack, bytes remaining: %u", remaining);
            this.task.isRunning = false;
            break;
        }
        battery_getInfo(&thisPtr->task.currentBatteryInfo);
        const bool previousIsCharging = thisPtr->task.previousBatteryInfo.isCharging;
        const bool currentIsCharging = thisPtr->task.currentBatteryInfo.isCharging;
        const float previousVoltage = thisPtr->task.previousBatteryInfo.voltage;
        const float currentVoltage = thisPtr->task.currentBatteryInfo.voltage;
        const float previousPercentage = thisPtr->task.previousBatteryInfo.percentage;
        const float currentPercentage = thisPtr->task.currentBatteryInfo.percentage;
        const float voltageDifference = (float) fabs((double) previousVoltage - currentVoltage);
        if (voltageDifference > thisPtr->voltageReadingMarginOfError) {
            if (thisPtr->task.voltageSampleChangedCount > thisPtr->voltageSampleMarginOfError) {
                const float percentageDifferance = (float) fabs((double) previousPercentage - currentPercentage);
                if (percentageDifferance > 0.0F) {
                    INFO("Percentage changed %f -> %f", previousPercentage, currentPercentage);
                    for (int i = 0; i < list_getSize(thisPtr->percentageChangedCallbacks); i++) {
                        const PercentageChangedCallback callback = list_getItem(thisPtr->percentageChangedCallbacks, i);
                        if (callback != NULL) {
                            callback(previousPercentage, currentPercentage);
                        }
                    }
                }
                thisPtr->task.voltageSampleChangedCount = 0;
                thisPtr->task.previousBatteryInfo.voltage = thisPtr->task.currentBatteryInfo.voltage;
                thisPtr->task.previousBatteryInfo.percentage = thisPtr->task.currentBatteryInfo.percentage;
            } else {
                thisPtr->task.voltageSampleChangedCount++;
            }
        }
        if (previousIsCharging != currentIsCharging) {
            INFO("IsCharging changed %s -> %s",
                 previousIsCharging ? "true" : "false", currentIsCharging ? "true" : "false");
            for (int i = 0; i < list_getSize(thisPtr->isChargingChangedCallbacks); i++) {
                const IsChargingChangedCallback callback = list_getItem(thisPtr->isChargingChangedCallbacks, i);
                if (callback != NULL) {
                    callback(previousIsCharging, currentIsCharging);
                }
            }
            thisPtr->task.previousBatteryInfo.isCharging = thisPtr->task.currentBatteryInfo.isCharging;
        }
        vTaskDelay(pdMS_TO_TICKS(BATTERY_TASK_POLL_MILLIS));
    }
    taskWatcher_notifyTaskStopped(
            /*taskName=*/BATTERY_TASK_NAME,
            /*shouldRestart=*/true,
            /*remainingStackBytes=*/remaining);
    vTaskDelete(thisPtr->task.handle);
}

public Error battery_init() {
    this.characteristics = new(esp_adc_cal_characteristics_t);
    this.batteryChannel = ADC1_CHANNEL_7;
    this.usbChannel = ADC1_CHANNEL_4;
    this.batterySamplesCount = 128;
    this.voltageReadingMarginOfError = 20.0F;
    this.voltageSampleMarginOfError = 6;
    this.percentageChangedCallbacks = list_create();
    this.isChargingChangedCallbacks = list_create();
    this.task.isRunning = true;

    TaskInfo taskInfo = {
            .name = BATTERY_TASK_NAME,
            .stackBytes = BATTERY_TASK_STACK_SIZE,
            .startFunction = battery_startBatteryTask
    };
    taskWatcher_registerTask(&taskInfo);
    battery_startBatteryTask();

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
    adc1_config_channel_atten(this.usbChannel, batteryAttenuation);
    esp_adc_cal_value_t calibrationUsed = esp_adc_cal_characterize(ADC_UNIT_1, batteryAttenuation, batteryBitsWidth,
                                                                   defaultVRef, this.characteristics);
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

    return ERROR_NONE;
}

public void battery_setSampleCount(const uint samplesCount) { this.batterySamplesCount = samplesCount; }

public uint battery_getSampleCount() { return this.batterySamplesCount; }

public float battery_getVoltage() {
    return battery_getVoltageFromReading(battery_getVoltageRawReading(this.batterySamplesCount),
                                         battery_isCharging());
}

public float battery_getPercentage() {
    return battery_getPercentageFromReading(battery_getVoltageRawReading(this.batterySamplesCount),
                                            battery_isCharging());
}

public bool battery_isCharging() {
    const float usbVoltage = battery_getIsChargingVoltage(battery_getUsbRawReading(8));
    return usbVoltage > 1000.0F;
}

public Error battery_getInfo(BatteryInfo *batteryInfo) {
    requireArgNotNull(batteryInfo);
    const uint reading = battery_getVoltageRawReading(this.batterySamplesCount);
    const bool isCharging = battery_isCharging();
    const float voltage = battery_getVoltageFromReading(reading, isCharging);
    const float percentage = battery_getPercentageFromReading(reading, isCharging);
    batteryInfo->isCharging = isCharging;
    batteryInfo->voltage = voltage;
    batteryInfo->percentage = percentage;
    return ERROR_NONE;
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
