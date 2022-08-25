# Notes

## Setup
When you first boot the camera, it should start as a Wifi access point that you connect to
(with a default password or open, undecided), from which, you then go to a web page that the
camera is serving (ie 192.168.0.1) that will contain wifi setup to an actual access point. 
This is akin to how some wifi printers do their SSID and password setup.
In order to re-enter this Access Point mode, you need to press and hold a "reset" button on 
the camera. This is useful when you have changed Wifi access point and can no longer access 
the camera, this reset button should be software based and only change the state containing 
wifi access point.

An account authentication would also be a part of this.

## Save Clips
The camera should have a button to signal that the user would like to save the last clip (however 
long we decide to set it, 15 or 30 seconds is good) similar to how some car dash-cams do this. 
These clips get saved into the SD card. 

## Over-The-Air Update Feature
It would be very cool to allow for Over-The-Air (OTA) software updates like how some routers do
this. This could be very complicated though but is a good flex of skills

## Custom Memory Allocator
This is a crazy idea I saw on YouTube, but we may want to use it if we see the benefit.

A custom memory allocator that can be more aware of memory pointers it allocates such as their size
and their state. We can even have all the actual allocation (`malloc`) up front and distribute the 
memory ourselves when needed, something akin to the JVM but without a GC.

An allocation doesn't actually allocate anything but instead assigns the pointer, and a free just 
makes it free to use later. We just need to maintain a list of all the pointers used and their state 
so there is a memory and performance cost for the benefit of predictability and control.

The benefits are currently small though so this is likely more costly than beneficial as of writing.

## Voltage Divider
Video was this https://www.youtube.com/watch?v=L321aqTvrLQ

## Fixing SD Module Card Detect Pin Signal
This pin is 0 when the SD card is NOT inserted but otherwise will just be floating
which will sometimes be close to 0, so not reliable to use, we use a "Pull-up resistor"
to fix this behavior to make it either 0 when card is NOT inserted and 1 otherwise,
thus allowing for reliable GPIO usage, the explanation is here:
https://learn.sparkfun.com/tutorials/pull-up-resistors/all
