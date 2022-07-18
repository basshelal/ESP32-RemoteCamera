file(GLOB LOGGER_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${LOGGER_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES log common)