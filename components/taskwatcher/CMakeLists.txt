file(GLOB TASK_WATCHER_SRC_FILES
        ./*.c ./*h)

idf_component_register(SRCS ${TASK_WATCHER_SRC_FILES}
        INCLUDE_DIRS "include"
        REQUIRES common logger freertos)