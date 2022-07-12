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

### MicroSD Card Module

[Manufacturer Site](https://www.adafruit.com/product/254)
[Digi-Key](https://www.digikey.co.uk/en/products/detail/adafruit-industries-llc/254/5761230)
[ThePiHut](https://thepihut.com/products/adafruit-microsd-card-breakout-board)

High quality feature-rich (somewhat expensive), MicroSD Card Module. MicroSD Card Modules are known
to be fussy about signal integrity, so I recommend using tight and secure wires, and soldering the 
included headers.

### Battery

TODO

## Software

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
Change the above appropriately, you will need to call `esp-export` once per terminal session before 
any ESP-IDF operations such as `idf.py build`

#### CLion

Add the environment script [`./setup-env.sh`](./setup-env.sh) to the current toolchain
so that CLion loads the path variables correctly, this solves the issue with unknown C, CXX and ASM compilers
as well as any "missing dependencies" problems that CMake would otherwise exclaim, 
see [JetBrains' guide](https://www.jetbrains.com/help/clion/how-to-create-toolchain-in-clion.html#env-scripts)
for how to do this and read more about ESP-IDF setup on CLion 
[here](https://www.jetbrains.com/help/clion/esp-idf.html).

##### Multi-root CMake Project

CLion is not yet capable of functioning with multiple root CMake projects. This project has 2 CMake roots,
[`./main/`](./main/) (the main component with the actual application) and [`./test/`](./test/) 
(the test component for running unit tests on device). The IDE will only *"be aware"* of 1 of these after
right-clicking "Load CMake Project" on [`./CMakeLists.txt`](./CMakeLists.txt) for `main` or
[`./test/CMakeLists.txt`](./test/CMakeLists.txt) for `test`. When one is loaded the other will have errors
in the IDE despite being able to build and run both from `idf.py` or `cmake` directly from the command-line.
It is a minor inconvenience that you will need to "Load CMake Project" when switching between editing either
of these components, but at least it makes sense as these are the only 2 *"applications"* that can be run 
on the device and only one can be run at a given time.