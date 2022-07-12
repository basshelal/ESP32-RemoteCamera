# WebClient

Here lies the code for developing the web client, any tools can be used as long as the 
final result is:
* Size optimal, SPIFFS has a limit, ideally we aim to be less than 500kB
* Minimal files used, because each file needed will have its own endpoint 
  on the ESP32 webserver meaning a SPIFFS read and respond for each file.
  The ideal case is 2 files, the HTML page and the JS single page application,
  anything else such as CSS or images (SVGs only) can be embedded in the JS.

The final result goes into the [`../webpages/`](../webpages) directory which is 
then used at build time to generate a SPIFFS image.