# Experiment11-SPI

## Overview
This experiment uses SPI protocol to shift data into a 75HC595 shift register and drive eight LEDs.  The Tiva board is the master device and the 595 is the slave.  The pin connections (Tiva - 595) are:  SS-RCLK, SCLK-SRCLK, and MOSI-SER.  (MISO is left open, since the slave sends no data).  The SCLK frequency is 1MHz.

The push buttons on the board control different LED patterns:  toggling (SW1),  counting (SW2), freeze value (SW1 & SW2),  shifting (none).  The software inserts a 100ms delay between each data transmission (to blink/animate the LEDs).

[![](http://img.youtube.com/vi/AN1EYTfsQEg/1.jpg)](https://youtu.be/AN1EYTfsQEg)<br>
[Watch the video](https://youtu.be/AN1EYTfsQEg)