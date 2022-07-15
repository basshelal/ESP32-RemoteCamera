# Internal Storage

Component that abstracts implementation of any internal storage functionality, that is
storage that is not on an external removable SD card, this is generally only:
* Reading and writing key-value pairs such as the Wi-Fi access point details (uses NVS)
* Fetching the HTML web pages for the UI (Uses SPIFFS)

Both use the ESP32's onboard flash storage and are thus independent from external storage
which is user-removable and thus unpredictable.