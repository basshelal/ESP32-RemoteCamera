# TODO

## Build System
* Add a build step to CMake to call `npm build` in the `webclient` directory to build
  the latest UI.

## Internal Storage
* Internal storage should get, put and delete key value pairs
* Internal storage should get files (such as index.html)

## WebClient
* UI prototype with live video player (probably using hls)
* Collapsible menu on the left side with all pages (using preact router),
  these are:
  * Live Feed (Home)
  * File Browser (Browse SD card files)
  * Settings
* Top left and right should have time, battery, log out/account management, storage space left
  Wi-Fi access point, IP address?, likely way less than this and only the bare essentials

## WebServer
* Begin Wi-Fi connection, see if we can use a static IP
* GET Battery voltage and percentage
* GET app routes responding to preact router, likely prefixed with `/app/*`
* GET View SD Card files
* GET/POST Delete SD Card files
* GET HLS live stream resource, this can initially be a public HLS for testing purposes.
  (The HLS encoding from Camera to WebServer may be complex and need its own component)

## Camera
* Take pictures and or videos, use the webserver and or SD card to check these
* Camera settings modification, if these are useful to user then user needs to access and set these
  permanently using internal storage
* Take pictures and store to SD Card
* Take videos and store to SD Card
* Record continuously live video and encode into HLS livestream, this may be complex enough to require
  its own component or file. The video data should probably live in memory as storage writes are expensive.