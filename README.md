# ESP32 Remote Camera

Work in progress

## Hardware

### Adafruit HUZZAH32 ESP32 Feather

[Manufacturer Site](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather)
[Digi-Key](https://www.digikey.co.uk/en/products/detail/adafruit-industries-llc/3591/8119805)
[ThePiHut](https://thepihut.com/products/adafruit-huzzah32-esp32-feather-board-pre-soldered-ada3591)

A very high quality, and feature-rich ESP32 board with a built-in Lithium Ion/Polymer battery
charger. It's on the expensive side, but the built-in battery charger (and the pre-soldered versions from
ThePiHut) make it worthwhile, especially if used for multiple or complex projects.

### Arducam 5MP OV5642 Mini SPI Camera Module

[Manufacturer Site](https://www.arducam.com/product/arducam-5mp-plus-spi-cam-arduino-ov5642/)
[Digi-Key](https://www.digikey.co.uk/en/products/detail/sparkfun-electronics/DEV-18440/15203664)
[ThePiHut](https://thepihut.com/products/5mp-ov5642-mini-spi-camera-module-for-arduino)

This is an excellent, high-quality, simple camera module that uses SPI (and I2C for commands) for communicating
with any microcontroller. You can attach any M12 Lens on the camera for different viewing angles.

### Battery

TODO

### SD-Card Module

TODO

## Software Development

### Software Setup

Follow the 
[ESP-IDF installation guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html)
before starting. 
You'll need to have the following in your `~/.bashrc` file in order to use ESP-IDF tools from
any shell: 
```bash
addPath() {
    export PATH="$*:$PATH"
}

addPath "/home/bassam/data/ESP32-IDF/xtensa-esp32-elf"
export IDF_PATH="/home/bassam/data/ESP-IDF/esp-idf"
export IDF_PYTHON_ENV_PATH="/usr/bin/python3.8"

alias esp-export=". $IDF_PATH/export.sh"
```
Change the above appropriately, you will need to call `esp-export` once before any ESP-IDF operations
such as `idf.py build`

### CLion

Add the environment script [`./setup-env.sh`](./setup-env.sh) to the current toolchain
so that CLion loads the path variables correctly, this solves the issue with unknown C, CXX and ASM compilers
as well as any "missing dependencies" problems that CMake would otherwise exclaim, 
see [JetBrains' guide](https://www.jetbrains.com/help/clion/how-to-create-toolchain-in-clion.html#env-scripts)
for how to do this and read more about ESP-IDF setup on CLion 
[here](https://www.jetbrains.com/help/clion/esp-idf.html).