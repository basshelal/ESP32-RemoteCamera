#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "app size:       $(printf "%'d" "$(du -b "$SCRIPT_DIR/build/ESP32-RemoteCamera.bin" | cut -f1)")"
echo "webclient size: $(printf "%'d" "$(du -b "$SCRIPT_DIR/webpages/index.html" | cut -f1)")"
