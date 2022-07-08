file(GLOB TEST_FILES
        ./*.c ./*.h
        )

idf_component_register(
        SRCS ${TEST_FILES}
        INCLUDE_DIRS "."
        PRIV_REQUIRES cmock unity common
)