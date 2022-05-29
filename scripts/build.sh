#!/usr/bin/env bash

. ./constants.sh
mkdir -p "$DEBUG_BUILD_DIR"
cmake -S "$PROJECT_ROOT_DIR" -B "$DEBUG_BUILD_DIR"
cmake --build "$DEBUG_BUILD_DIR"