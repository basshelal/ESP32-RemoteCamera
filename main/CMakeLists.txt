set(SRC_FILES
        main.c
        utils.h utils.c
        services/wifi.h services/wifi.c
        services/webserver.h services/webserver.c
        secrets.h)

idf_component_register(SRCS ${SRC_FILES} INCLUDE_DIRS ".")
