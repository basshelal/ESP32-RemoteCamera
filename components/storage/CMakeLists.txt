file(GLOB STORAGE_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${STORAGE_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES common logger spiffs fatfs nvs_flash sdmmc)