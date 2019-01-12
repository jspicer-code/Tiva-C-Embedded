# Experiment5-UART

## Overview
This experiment sets up UART5 on the Tiva board and connects to the PC using an FTDI RS-232 to 3.3v converter cable. The embedded application transmits a selection menu to the PC that allows the user to choose one of eight colors to display on the tri-color LED. PuTTY is used as the terminal program on the PC. 

A scope probe was hooked up to the TX pin (PE5) to view the waveform of the characters transmitted from the board. To see a more regular, periodic signal, the embedded app transmits the letter 'U' repeatedly when SW1 is held down and a menu selection is made. This appears on the oscilloscope as a series of alternating pulses.

[![](http://img.youtube.com/vi/AObu1TXMY2k/1.jpg)](https://youtu.be/AObu1TXMY2k)<br>
[Watch the video](https://youtu.be/AObu1TXMY2k)

## Circuit
This experiment uses the TTL-232R-3V3-WE USB to TTL serial cable (wire end version) made by FTDI Chip.  The data sheet is available on FTDI's web site, currently:  https://www.ftdichip.com/Support/Documents/DataSheets/Cables/DS_TTL-232R_CABLES.pdf.  The cable costs around $17 (US) and can be found on Amazon, Mouser, etc.  It is available with or without a header on the wire ends.   

The circuit diagram below shows the connections between the EK-TM4C123GXL and the cable wire ends.

![Experiment5-UART](Experiment5-UART-circuit.png)  