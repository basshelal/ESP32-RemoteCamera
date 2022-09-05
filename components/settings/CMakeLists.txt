file(GLOB SETTINGS_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${SETTINGS_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES log common nvs_flash)