file(GLOB WIFI_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${WIFI_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES common logger storage esp_wifi esp_event nvs_flash)