# Experiment10-MultiplexedDisplay

## Overview
This experiment tests the implementation of a multiplexed, 7-segment display with four digits.  The software switches each of the four digits on and off every millisecond, giving the appearance that all four are always on.  

The display has four common anodes (one for each display) and eight segment pins.  The anodes are switched on/off via a transistor (NPN, 2N2222A) in a common collector (emitter follower) configuration, that supplies roughly constant voltage and high current gain to the display.  The base pins of the transistors are connected to four GPIO pins.  The segment pins of the display are connected to a BCD to 7-segment decoder (74LS47), and the decoder inputs are connected to four GPIO.

The software utilizes one of the Timer blocks of the TM4C123 that raises and interrupt every millisecond.  The ISR keeps track of which digit needs refreshing (in round-robin fashion), outputs the digit's BCD value to the decoder, and then turns the digit on for 1 msec. while turning all the other digits off.

The application runs in one of two modes, depending on the position of the switch on the board:  counter or ADC output.  The counter mode shows the value of a counter that increments every second, and ADC mode shows the digital output (0-4095) from an ADC input connected to a potentiometer on the board.  

[![](http://img.youtube.com/vi/v5YLxzM2EFQ/1.jpg)](https://youtu.be/msRxsZsFCN4)<br>
[Watch the video](https://youtu.be/msRxsZsFCN4)
