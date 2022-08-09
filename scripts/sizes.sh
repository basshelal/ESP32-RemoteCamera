#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

APP_SIZE=$(du -b "$SCRIPT_DIR/../build/ESP32-RemoteCamera.bin" | cut -f1)
APP_SIZE_FORMATTED=$(printf "%'d" "$APP_SIZE")
WEB_SIZE=$(du -b "$SCRIPT_DIR/../webpages/index.html" | cut -f1)
WEB_SIZE_FORMATTED=$(printf "%'d" "$WEB_SIZE")
TOTAL_SIZE=$(($APP_SIZE + $WEB_SIZE))
TOTAL_SIZE_FORMATTED=$(printf "%'d" "$TOTAL_SIZE")
echo "app size:    $APP_SIZE_FORMATTED"
echo "web size:    $WEB_SIZE_FORMATTED"
echo "total:       $TOTAL_SIZE_FORMATTED"