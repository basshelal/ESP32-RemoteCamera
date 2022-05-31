file(GLOB SRC_FILES
        ./*.c ./*.h
        services/*.c
        services/*.h)

idf_component_register(SRCS ${SRC_FILES} INCLUDE_DIRS ".")
