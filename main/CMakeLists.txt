file(GLOB SRC_FILES
        ./*.c ./*.h
        peripherals/*.c peripherals/*.h
        services/*.c services/*.h
        )

idf_component_register(SRCS ${SRC_FILES} INCLUDE_DIRS ".")

# Create a SPIFFS image from the contents of the 'webpages' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
spiffs_create_partition_image(storage ../webpages FLASH_IN_PROJECT)