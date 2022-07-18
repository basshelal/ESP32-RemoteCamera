cmake_minimum_required(VERSION 3.5)

# Include the components directory of the main application:
set(EXTRA_COMPONENT_DIRS components/)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32-RemoteCamera)

add_custom_command(TARGET app
        POST_BUILD
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/webclient)