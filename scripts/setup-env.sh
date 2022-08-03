#!/usr/bin/env bash

# Needs to be sourced before other scripts as in . ./constants.sh

addPath() {
    export PATH="$*:$PATH"
}

addPath "/home/bassam/data/ESP32-IDF/xtensa-esp32-elf"
export IDF_PATH="/home/bassam/data/ESP-IDF/esp-idf"
export IDF_PYTHON_ENV_PATH="/usr/bin/python3.8"

. $IDF_PATH/export.sh

export PROJECT_ROOT_DIR="./"
export DEBUG_BUILD_DIR="$PROJECT_ROOT_DIR/cmake-build-debug/"
export BUILD_DIR="$PROJECT_ROOT_DIR/build/"