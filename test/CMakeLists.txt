# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.5)

# Include the components directory of the main application:
set(EXTRA_COMPONENT_DIRS ../components/)

# Set the components to include the tests for.
# This can be overriden from CMake cache:
# - when invoking CMake directly: cmake -D TEST_COMPONENTS="xxxxx" ..
# - when using idf.py: idf.py -T xxxxx build
#
set(TEST_COMPONENTS common logger storage)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32-RemoteCamera_test)

add_custom_command(TARGET app
        POST_BUILD
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../webclient)