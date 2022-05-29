# ESP32 Remote Camera

Work in progress

## Configuring

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