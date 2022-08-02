file(GLOB WEBSERVER_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${WEBSERVER_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES common logger wifi storage esp_http_server)