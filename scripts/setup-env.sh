#!/usr/bin/env bash

# Needs to be sourced before other scripts as in . ./constants.sh

addPath() {
    export PATH="$*:$PATH"
}

export IDF_PATH="$HOME/data/ESP-IDF/esp-idf"

. $IDF_PATH/export.sh

export PROJECT_ROOT_DIR="./"
export DEBUG_BUILD_DIR="$PROJECT_ROOT_DIR/cmake-build-debug/"
export BUILD_DIR="$PROJECT_ROOT_DIR/build/"