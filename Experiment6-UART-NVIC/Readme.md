# Experiment6-UART-NVIC

## Overview
This experiment is a continuation of the previous experiment that tests out the UART on the Tiva board.   Instead of using an RS232 to TTL converter cable, it uses a Maxim 3232 transceiver chip with external tantalum (0.1uF) capacitors.

The code now uses an interrupt handler instead of polling the UART for received characters.  It has also been refactored to use a state machine, and the hardware functions were improved to eliminate redundancy and create a better level of abstraction for the calling program(s).

[![](http://img.youtube.com/vi/Sd4p9AEHWPo/1.jpg)](https://youtu.be/Sd4p9AEHWPo)<br>
[Watch the video](https://youtu.be/Sd4p9AEHWPo)
