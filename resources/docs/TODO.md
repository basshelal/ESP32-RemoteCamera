# TODO

## Log
* Implement and add LogList which will be used by the webclient /log/ endpoint which will use
  a websocket to return log lines
* We should add an RGB LED capable of displaying multiple colors and effects to signal and detail
  errors and progress such as WiFi connecting, failed, awaiting setup etc but only for more critical
  and necessary signals, combining color, brightness, and effects (such as flashing with speed or even
  patterns) means we can hardcode a (very) large number of signals, such a system could be used
  on future projects as well

## Battery
* Clean component structure
* Battery voltage and percentage info

## WebServer/WebClient
* Battery API endpoint
* LogList websocket

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

## Final Improvements
* Size reductions
  * Try to reduce HLS.js by making it tree-shakeable if possible, this is the easiest way to have 
    a large effect without having to understand the inner implementation