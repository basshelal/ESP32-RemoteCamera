#!/usr/bin/env bash

echo "app size:       $(printf "%'d" "$(du -b "build/ESP32-RemoteCamera.bin" | cut -f1)")"
echo "webclient size: $(printf "%'d" "$(du -b "webpages/index.html" | cut -f1)")"
