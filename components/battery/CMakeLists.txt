file(GLOB BATTERY_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${BATTERY_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES common logger esp_adc_cal freertos)