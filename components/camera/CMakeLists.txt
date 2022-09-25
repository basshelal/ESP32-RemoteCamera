file(GLOB CAMERA_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${CAMERA_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES common logger driver)