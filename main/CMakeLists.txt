set(SRC_FILES
        main.c
        utils.h utils.c)

idf_component_register(SRCS ${SRC_FILES} INCLUDE_DIRS ".")
