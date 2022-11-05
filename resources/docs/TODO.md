# TODO

## WebServer
* URI Parser to decode URIs with `%20` instead of space for example
  use this library https://github.com/uriparser/uriparser
  and add libraries like here 
  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html#using-third-party-cmake-projects-with-components
* GET View SD Card files
* GET/POST Delete SD Card files

## Camera
* Camera settings modification over webserver, settings should be saved permanently on SPIFFS
* Take pictures and store to SD Card
* Take videos and store to SD Card, videos are in MJPEG format, essentially, moving pictures
* Try to implement concurrent multi-frame capture and read by taking advantage of the Arducam's FIFO
  so that we can capture 4 (or more) frames and while waiting for capture to finish, start reading the
  FIFO to minimize time waiting for capture to finish, this may not give huge framerate wins though and
  will likely add a lot more code complexity because of concurrency management etc

## WiFi
* WebServer calls WiFi to see if we can get an internet connection:
  Wifi checks if we have credentials stored to connect with:
  true then try to connect with them until success or fail
  false (or after enough retries) then connect into setup-mode where ESP32 is a
  Wifi access point (and also station) where you can visit the page to view
  logs and set-up connection to diagnose issues
  wifi responds, ok if we connect no problem, otherwise maybe no credentials exist on storage
  thus tells webserver so and then webserver tells wifi to start in STA mode to begin the 
  setup page server
* The setup page may have to be in mixed mode, when on the page, scan for Wifi access points for
  easier user experience, when connected we can check an internet connection to ensure all works
  an internet connection is generally required or at least preferred, maybe also allow an option
  for no internet, ie, remote camera that is connected to via its own network
* Allow (advanced) users to choose and store their desired IP address

## Generic
* Allow `List` to not shift or change shape when items are removed to allow for a concurrent 
  read and modification list, so that you can loop over the list and remove items without problems,
  maybe also add a function to reshape if desired, gaps in the list (removed items) will just be `NULL`s
* Create a common `Error` type which every function returns, this enum type contains all possible errors
  and is declared in the `Common` component, it's the best solution to having shared errors
* Modify all functions to follow improved coding style, all functions return either an ErrorType
  or `void` and nothing else, any returns are done through pointer parameters (stack pointers are 
  the easy solution here) to allow for easy "exception handling" and even multiple return types
* All modules with `init()` functions need to be made multi-call safe using a simple `bool` `isInitialized`

## Log
* We should add an RGB LED capable of displaying multiple colors and effects to signal and detail
  errors and progress such as WiFi connecting, failed, awaiting setup etc but only for more critical
  and necessary signals, combining color, brightness, and effects (such as flashing with speed or even
  patterns) means we can hardcode a (very) large number of signals, such a system could be used
  on future projects as well
* Webpage could allow for colored lines depending on log level
* Webpage should autofocus to log
* Log could be black
* Line count and scroll to bottom button under the log
* Websocket status and websocket reconnect at the bottom under log
* We can check websocket status by running a polling function that uses the socket?

## Setup
* Setup includes a reset button (and software switch) to reset device to factory defaults
* Factory default means first time setup page which contains Wi-Fi information setup
  and account setup

## WebClient
* Implement authentication system on webclient

## Battery
* See if we can get better information, such as more accurate battery reading (directly from BAT pin)
  and being aware of USB presence to know if we are charging or not (from USB pin)
* Better battery percentage function that converts the voltage lifecycle to a more linear percentage,
  this can only be done by sampling the battery's life and finding the curve and correcting it to be 
  linear

## OTA Updates
* Investigate and consider OverTheAir _"firmware"_ updates

## Hardware Packaging
* Consider making hardware package semi-permanent to see how far we can go with authentic development
  experience as if we are presenting a prototype. This is also useful for archiving and documenting purposes.

## Final Improvements
* Size reductions