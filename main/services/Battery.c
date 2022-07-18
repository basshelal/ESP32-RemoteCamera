#include "Battery.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_log.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1079 // from espefuse.py adc_info
#define NO_OF_SAMPLES   256

private esp_adc_cal_characteristics_t *adc_chars;
private const adc1_channel_t channel = ADC1_CHANNEL_7;
private const adc1_channel_t batteryChannel = ADC1_CHANNEL_0;
private const adc1_channel_t usbChannel = ADC1_CHANNEL_5;
private const adc_bits_width_t width = ADC_WIDTH_BIT_12;
private const adc_atten_t atten = ADC_ATTEN_DB_11;

public esp_err_t battery_init() {
    // Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    adc1_config_channel_atten(batteryChannel, atten);
    adc1_config_channel_atten(usbChannel, atten);

    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);

    return ESP_OK;
}

public uint32_t battery_raw(const int samplesCount) {
    uint32_t adc_reading = 0;
    // Multisampling
    for (int i = 0; i < samplesCount; i++) {
        adc_reading += adc1_get_raw(channel);
    }
    adc_reading /= samplesCount;
    return adc_reading;
}

public float battery_voltage() {
    uint32_t raw = battery_raw(NO_OF_SAMPLES);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    const float result = ((float) voltage) / 3300.0F;

    ESP_LOGI("Battery", "Raw: %d\tVoltage: %dmV", raw, voltage);

    return result;
}

public float battery_percentage() {
    uint32_t adc_reading = 0;
    // Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(channel);
    }
    adc_reading /= NO_OF_SAMPLES;
    // Convert adc_reading to voltage in mV
    const float myResult = (((float) adc_reading) / 4095.0F) * 2.0F * 3.3F * 1.1F;

    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    ESP_LOGI("Battery", "Raw: %d\tVoltage: %dmV, myResult: %.2fV", adc_reading, voltage, myResult);

    return (myResult / 4.2F) * 100.0F;
}

private char buffer[512];
public const char *battery_text() {
    uint32_t averagedRaw = 0;
    // Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        averagedRaw += adc1_get_raw(channel);
    }
    averagedRaw /= NO_OF_SAMPLES;

    // TODO: 17-Jul-2022 @basshelal: For more realtime accurate battery readings as well as determining if we are
    //  plugged in and charging the battery see: https://learn.adafruit.com/adafruit-huzzah32-esp32-feather?view=all#power-pins-2816344
    //  If we connect and read some of these pins, namely BAT and USB we can determine the current power state
    //  the ADC battery reading will be slightly higher when charging and so doesn't correctly represent the
    //  battery's voltage, the BAT pin is the most accurate one

    // TODO: 17-Jul-2022 @basshelal: Find a nice linear battery percentage function for this 3.7V battery because
    //  it doesn't change voltage linearly it hangs at 3.7V most of its life peaking at ~4.2V and dying at ~3.2V
    //  for this we can log all of the voltages throughout its life into a CSV and find the curve and see if we can
    //  reverse it to get a more linear change if possible

    // TODO: 18-Jul-2022 @basshelal: Charging seems to add an additional .05V to the battery

    const uint32_t espVoltage = esp_adc_cal_raw_to_voltage(averagedRaw, adc_chars);
    const float calculatedVoltage = ((float) espVoltage * 2.0F) / 1000.0F;

    snprintf(buffer, 512, "raw: %i espVoltage: %i calculatedVoltage: %.2fV",
             averagedRaw, espVoltage, calculatedVoltage);
    return buffer;
}