# Experiment12-SPI-MultiplexedDisplay

## Overview
This experiment combines the previous two experiments by using SPI and the 595 shift register to multiplex a four digit seven-segment display.  The advantages of this circuit compared to the previous one are that 1) it requires only 3 pins on the TM4C123 instead of eight, and 2) it is possible to supply the transistors with 5V instead of 3.3V, and therefore deliver more current to make the the LEDs brighter.

Four of the eight outputs from the 74HC595 are used to provide the BCD inputs to a 74LS47 decoder that controls which LED segments are turned on.  The other four outputs are used to switch on the NPN transistors that supply current to the common anodes of the display.  The transistors are configured as emitter followers, and the 5V outputs of the 595 mean that 4.3V are applied to the common anodes.  At this voltage it was necessary to limit the peak current through the LEDs to about 20 mA using 82 ohm resistors.  The video goes into some detail about how the peak current was calculated.

To demonstrate the circuit, potentiometers were hooked up to two of the board's ADCs and their digital outputs (0-4095) sent to the display.  A three-way switch controls which ADC output is displayed or blanks out the display if set to the middle position.

[![](http://img.youtube.com/vi/KHmJOviVY9Y/2.jpg)](https://youtu.be/KHmJOviVY9Y)<br>
[Watch the video](https://youtu.be/KHmJOviVY9Y)