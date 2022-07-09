file(GLOB COMMON_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${COMMON_SRC_FILES}
        INCLUDE_DIRS "include")