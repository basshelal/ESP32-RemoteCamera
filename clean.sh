#!/usr/bin/env bash

# Clean build residuals and artifacts

. ./setup-env.sh >/dev/null
echo "Deleting $DEBUG_BUILD_DIR"
rm -r "$DEBUG_BUILD_DIR" 2>/dev/null
echo "Deleting $BUILD_DIR"
rm -r "$BUILD_DIR" 2>/dev/null