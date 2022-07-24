# TODO

## WebClient
* Basic prototype with Preact + MaterializeCSS

## Battery
* Clean component structure
* Battery voltage and percentage info

## WebServer
* Begin Wi-Fi connection, see if we can use a static IP
* Add CJSON library for easy web request handling
* GET Battery voltage and percentage
* GET app routes responding to preact router, likely prefixed with `/app/*`
* GET HLS live stream resource, this can initially be a public HLS for testing purposes.
  (The HLS encoding from Camera to WebServer may be complex and need its own component)

## External Storage
* Check for existing and valid external storage
* External storage stats, remaining/used space, file count, dir count etc
* Read dirs and files (using POSIX APIs)
* Create dirs and files (using POSIX APIs)
* Write (update and delete) dirs and files (using POSIX APIs) 

## WebServer
* GET View SD Card files
* GET/POST Delete SD Card files

## Camera
* Take pictures and or videos, use the webserver and or SD card to check these
* Camera settings modification, if these are useful to user then user needs to access and set these
  permanently using internal storage
* Take pictures and store to SD Card
* Take videos and store to SD Card
* Record continuously live video and encode into HLS livestream, this may be complex enough to require
  its own component or file. The video data should probably live in memory as storage writes are expensive.

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