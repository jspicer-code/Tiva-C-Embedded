# Experiment7-ADC

## Overview
This experiment tests out the ADC on the Tiva board.  It is the basic implementation of a volt meter that samples the voltage on a 10K potentiometer which is connected between 3.3V and ground.  The voltage sample is converted to decimal and displayed on two seven-segment displays, up to 1/10th of a volt.  

The ADC is software triggered and uses SS3 and AIN0 (pin PE3).  The displays are driven by two ULN2803A Darlington transistor arrays, one per display.  One GPIO pin is used for each segment of the display and connected to the input of a transistor.  Turning on the GPIO pin causes current to sink through the LED segment into the open collector output of the transistor.

[![](http://img.youtube.com/vi/BE-GWVR0DsE/1.jpg)](https://youtu.be/BE-GWVR0DsE)<br>
[Watch the video](https://youtu.be/BE-GWVR0DsE)
