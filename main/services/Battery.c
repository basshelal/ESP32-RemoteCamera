#include "Battery.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   128

static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t channel = ADC1_CHANNEL_7;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;

public esp_err_t battery_init() {
    // Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);

    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);

    return ESP_OK;
}

public esp_err_t battery_read() {
    uint32_t adc_reading = 0;
    // Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(channel);
    }
    adc_reading /= NO_OF_SAMPLES;
    // Convert adc_reading to voltage in mV
    const float myResult = (((float) adc_reading) / 4095.0F) * 2 * 3.3 * 1.1;

    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    ESP_LOGI("Battery", "Raw: %d\tVoltage: %dmV, myResult: %.2fV", adc_reading, voltage, myResult);

    return ESP_OK;
}