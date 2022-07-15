file(GLOB INTERNAL_STORAGE_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${INTERNAL_STORAGE_SRC_FILES}
        INCLUDE_DIRS "include")