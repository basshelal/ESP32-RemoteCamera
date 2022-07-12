file(GLOB TEST_UTILS_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${TEST_UTILS_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES unity)