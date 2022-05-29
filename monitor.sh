#!/usr/bin/env bash

# Monitor serial output from device

. ./setup-env.sh 2>/dev/null

#minicom -b 115200 -o -D /dev/ttyUSB0

idf.py monitor