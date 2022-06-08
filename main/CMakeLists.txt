file(GLOB SRC_FILES
        ./*.c ./*.h
        peripherals/*.c peripherals/*.h
        services/*.c services/*.h
        utils/*.c utils/*.h)

idf_component_register(SRCS ${SRC_FILES} INCLUDE_DIRS ".")
