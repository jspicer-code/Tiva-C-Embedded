# Experiment14-LCD

## Overview
This experiment uses the EK-TM4C123GXL to drive two LCD displays based on the Hitachi HD44780 controller spec.  One is a 16x2 display w/backlight (Techstar TS1620-21/B) and the other is a 16x1 reflective display (Lumex LCM-S01601DSR).  The TM4C123 is configured to use GPIO (digital) pins to transfer data to the displays using 4-bit data bus mode.

A two-way switch on the breadboard controls which display receives the input (cursor) focus.  Cursor movement and character entry is controlled by an additional 5-position switch, which has directions for up, down, left, right, and center.  The cursor can be moved around via the up, down, left, and right positions.  Pressing the 5-position switch vertically downward (center position) causes the cursor to blink, and a new character can be selected by moving the switch in the other directions:  left/right traverses the alphabet and symbols, whereas up/down toggles between upper and lower case and a different set of symbols.  Pressing the center switch down again locks in the character. 

Holding down the on-board switch, SW2, causes the both displays to side scroll together, like a single 32x2 marquee. 

The LCD driver is split into lower (raw) and higher level components.  The raw driver handles communication with the HD44780 controller, and implements eight or so of its instruction codes, such as Function Set, Set DDRAM Address, Entry Mode Set, etc.  The higher level LCD driver has more abstract functions for writing text and moving the cursor around, and handles the differences between a 16x1 and 16x2 display, such as selecting DDRAM addresses.

[![](https://i.ytimg.com/vi/5TJ8u1CpPAs/default.jpg)](https://youtu.be/5TJ8u1CpPAs)<br>
[Watch the video](https://youtu.be/5TJ8u1CpPAs)


![Experiment14-LCD](Experiment14-LCD-circuit.png)  