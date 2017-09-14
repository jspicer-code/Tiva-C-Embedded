# Experiment5-UART

## Overview
This experiment sets up UART5 on the Tiva board and connects to the PC using an FTDI RS-232 to 3.3v converter cable. The embedded application transmits a selection menu to the PC that allows the user to choose one of eight colors to display on the tri-color LED. PuTTY is used as the terminal program on the PC. 

A scope probe was hooked up to the TX pin (PE5) to view the waveform of the characters transmitted from the board. To see a more regular, periodic signal, the embedded app transmits the letter 'U' repeatedly when SW1 is held down and a menu selection is made. This appears on the oscilloscope as a series of alternating pulses.

[![](http://img.youtube.com/vi/AObu1TXMY2k/1.jpg)](https://youtu.be/AObu1TXMY2k)<br>
[Watch the video](https://youtu.be/AObu1TXMY2k)
