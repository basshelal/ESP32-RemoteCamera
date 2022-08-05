cmake_minimum_required(VERSION 3.5)

# Include the components directory of the main application:
set(EXTRA_COMPONENT_DIRS components/)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32-RemoteCamera)

set(MY_COMPILE_OPTIONS -Wno-unused-function -Wno-discarded-qualifiers -Wno-unused-variable -Wno-unused-but-set-variable)
idf_build_set_property(COMPILE_OPTIONS "${MY_COMPILE_OPTIONS}" APPEND)

add_custom_command(TARGET app
        POST_BUILD
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/webclient)

add_custom_command(TARGET app
        POST_BUILD
        COMMAND ./sizes.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts)