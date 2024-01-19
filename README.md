# CallWriter
Write some text in the waterfall display of a (web) SDR receiver.
Transmit the DAC output noise by the mic in USB or LSB, AM is also possible.

## Arduino XIAO board
This version of my CallWriter is using the [Seeed XIAO cpu (SAMD21G18, ARM® Cortex®-M0+](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html)

## CallWriter V1
This was the first version that generate the 16 DDS sinus signals to create a 16x11 character font.

## CallWriter V2
This version did add a band-pass filter on the DAC output signal to get a more smood signal with the dots 
flipping on and off.

## CallWriter V3
The text transmitted can be changed by the USB serial input.

## Picture
Received at HackGreen:
![SDR receiver hackgreensdr.org at 40m](/images/hackgreen.png)
http://hackgreensdr.org:8901/
