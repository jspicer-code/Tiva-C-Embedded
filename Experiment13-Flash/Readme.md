# Experiment13-Flash

## Overview
This experiment tests storing data in the TM4C123's Flash memory.  Pressing one of the on-board buttons cycles through eight different LED colors, and pressing the other on-board button stores the color selection into Flash memory.  When the device is reset, it loads the saved color from Flash and sets the LEDs to this color.

A hardware abstraction module was written that has operations to Erase, Write, and Read from Flash memory.  The region of Flash memory for storing user data was chosen to be the upper half of the 256K total, i.e. starting at the 128K boundary.  The video goes into detail about how the HAL functions were written and the register accesses involved.

[![](http://img.youtube.com/vi/coztn7F4jbM/1.jpg)](https://youtu.be/coztn7F4jbM)<br>
[Watch the video](https://youtu.be/coztn7F4jbM)