# Experiment4-SysTick

## Overview
This experiment tests the SysTick timer and PLL on the EV-TM4C123GLX board.  The PLL is enabled and configured to run the bus clock at 80MHz.  The preset for the SysTick timer is taken from the value of 8 GPIO inputs that are connected to an 8-pin DIP switch.  The binary value on the DIP switches is converted to decimal and multiplied by 10ms.  The maximum timer delay is therefore 255 * 10ms = 2.55 seconds.  

Three LEDs are connected to three GPIO output ports and turned on/off after the SysTick timer counts down to zero and rolls over.  The LEDs turn on/off in a shifting pattern from right to left.  The red, onboard LED is also made to blink at the same rate.  

[![](http://img.youtube.com/vi/_GmFn_izdOk/1.jpg)](https://youtu.be/_GmFn_izdOk)<br>
[Watch the video](https://youtu.be/_GmFn_izdOk)
