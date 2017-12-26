# Tiva C Series ARM development

This repository contains projects and experiments developed for the EK-TM4C123GXL ARM evaluation board.

# Projects

## Fan Controller
This project is a fan controller system.  It features PWM speed control, a numeric display, and a serial console to the PC. The PWM speed control interfaces with standard 4-wire PWM fans.  It supports thermal and manual control modes and RPM measurement of the fan's tachometer signal.  The display shows the fan speed (0-100), RPM, or current temperature, and the serial console interface provides a menu-driven UI for configuring the temperature control settings.

Hardware features used on the Tiva TM4C123 board include GPIO, ADC, PWM, SPI, Timers, UART, PLL, SysTick, and Flash storage.   The firmware was written in C using Keil Microvision and uses direct register access to control the hardware.

Users can configure the system for linear thermal control by setting lower and upper temperature limits.  When the environment temperature reaches the lower limit, the fan will turn on at the slowest speed and increase linearly up to the maximum speed, proportional to the higher temperature limit.  Above the high limit the fan will run at full speed.   Thermostatic control can be configured by setting the low and high temperature limits the same, which turns the fan on at full speed when the temperature is the same or above the limit.

This project evolved from experiments 1-13 and contains some or all of the features from each.

[![](http://img.youtube.com/vi/YEEtr2Ipl0c/3.jpg)](https://youtu.be/YEEtr2Ipl0c)<br>
[Watch the video](https://youtu.be/YEEtr2Ipl0c)

# Experiments

## Experiment13-Flash
This experiment tests storing data in the TM4C123's Flash memory.  Pressing one of the on-board buttons cycles through eight different LED colors, and pressing the other on-board button stores the color selection into Flash memory.  When the device is reset, it loads the saved color from Flash and sets the LEDs to this color.

A hardware abstraction module was written that has operations to Erase, Write, and Read from Flash memory.  The region of Flash memory for storing user data was chosen to be the upper half of the 256K total, i.e. starting at the 128K boundary.  The video goes into detail about how the HAL functions were written and the register accesses involved.

[![](http://img.youtube.com/vi/coztn7F4jbM/1.jpg)](https://youtu.be/coztn7F4jbM)<br>
[Watch the video](https://youtu.be/coztn7F4jbM)

## Experiment12-SPI-MultiplexedDisplay
This experiment combines the previous two experiments by using SPI and the 595 shift register to multiplex a four digit seven-segment display.  The advantages of this circuit compared to the previous one are that 1) it requires only 3 pins on the TM4C123 instead of eight, and 2) it is possible to supply the transistors with 5V instead of 3.3V, and therefore deliver more current to make the LEDs brighter.

Four of the eight outputs from the 74HC595 are used to provide the BCD inputs to a 74LS47 decoder that controls which LED segments are turned on.  The other four outputs are used to switch on the NPN transistors that supply current to the common anodes of the display.  The transistors are configured as emitter followers, and the 5V outputs of the 595 mean that 4.3V are applied to the common anodes.  At this voltage it was necessary to limit the peak current through the LEDs to about 20 mA using 82 ohm resistors.  The video goes into some detail about how the peak current was calculated.

To demonstrate the circuit, potentiometers were hooked up to two of the board's ADCs and their digital outputs (0-4095) sent to the display.  A three-way switch controls which ADC output is displayed or blanks out the display if set to the middle position.

[![](http://img.youtube.com/vi/KHmJOviVY9Y/2.jpg)](https://youtu.be/KHmJOviVY9Y)<br>
[Watch the video](https://youtu.be/KHmJOviVY9Y)

## Experiment11-SPI
This experiment uses SPI protocol to shift data into a 75HC595 shift register and drive eight LEDs.  The Tiva board is the master device and the 595 is the slave.  The pin connections (Tiva - 595) are:  SS-RCLK, SCLK-SRCLK, and MOSI-SER.  (MISO is left open, since the slave sends no data).  The SCLK frequency is 1MHz.

The push buttons on the board control different LED patterns:  toggling (SW1),  counting (SW2), freeze value (SW1 & SW2),  shifting (none).  The software inserts a 100ms delay between each data transmission (to blink/animate the LEDs).

[![](http://img.youtube.com/vi/AN1EYTfsQEg/1.jpg)](https://youtu.be/AN1EYTfsQEg)<br>
[Watch the video](https://youtu.be/AN1EYTfsQEg)

## Experiment10-MultiplexedDisplay
This experiment tests the implementation of a multiplexed, 7-segment display with four digits.  The software switches each of the four digits on and off every millisecond, giving the appearance that all four are always on.  

The display has four common anodes (one for each display) and eight segment pins.  The anodes are switched on/off via a transistor (NPN, 2N2222A) in a common collector (emitter follower) configuration, that supplies roughly constant voltage and high current gain to the display.  The base pins of the transistors are connected to four GPIO pins.  The segment pins of the display are connected to a BCD to 7-segment decoder (74LS47), and the decoder inputs are connected to four GPIO.

The software utilizes one of the Timer blocks of the TM4C123 that raises and interrupt every millisecond.  The ISR keeps track of which digit needs refreshing (in round-robin fashion), outputs the digit's BCD value to the decoder, and then turns the digit on for 1 msec. while turning all the other digits off.

The application runs in one of two modes, depending on the position of the switch on the board:  counter or ADC output.  The counter mode shows the value of a counter that increments every second, and ADC mode shows the digital output (0-4095) from an ADC input connected to a potentiometer on the board.  

[![](http://img.youtube.com/vi/msRxsZsFCN4/1.jpg)](https://youtu.be/msRxsZsFCN4)<br>
[Watch the video](https://youtu.be/msRxsZsFCN4)

## Experiment9-Thermistor
This experiment tests out an NTC thermistor temperature sensor.  The thermistor is placed in series with a 10K resistor, and an ADC input is placed at the divider.  Using a temperature/resistance table (one table for Fahrenheit and another for Celsius) from the vendor, the ADC input is mapped to the temperature using a binary search.  The temperature values are then written to the UART and displayed on the PC terminal.

[![](http://img.youtube.com/vi/v5YLxzM2EFQ/3.jpg)](https://youtu.be/v5YLxzM2EFQ)<br>
[Watch the video](https://youtu.be/v5YLxzM2EFQ)

## Experiment8-PWM-Timers
This experiment is the implementation of a fan speed controller that tests out the PWM and timer block features of the Tiva board.  It controls a 12V fan that has a PWM input and tachometer output.  To vary the fan speed, a potentiometer was connected to one of the ADC inputs on the board, and the ADC sample converted to a duty cycle.  Turning the pot in one direction narrows the pulse and slows down the fan, and in the other direction it widens the pulse and speeds up the fan.

The fan's tachometer output is a square wave.  Every two pulses represent one revolution of the fan.  To count the pulses, an input edge counter (timer) was set up on the board to count the rising edges, and another timer (count-down) was set up to raise an interrupt every second.  Inside the ISR, the tach pulse count is read and stored in a shared variable, and then both timers are reset to do this again every second.  The main program reads the stored pulse count, converts it to RPM, and then writes the value to the UART where it is displayed on the PC.

[![](http://img.youtube.com/vi/nJTgNQMfvj8/2.jpg)](https://youtu.be/nJTgNQMfvj8)<br>
[Watch the video](https://youtu.be/nJTgNQMfvj8)

## Experiment7-ADC
This experiment tests out the ADC on the Tiva board.  It is the basic implementation of a volt meter that samples the voltage on a 10K potentiometer which is connected between 3.3V and ground.  The voltage sample is converted to decimal and displayed on two seven-segment displays, up to 1/10th of a volt.  

The ADC is software triggered and uses SS3 and AIN0 (pin PE3).  The displays are driven by two ULN2803A Darlington transistor arrays, one per display.  One GPIO pin is used for each segment of the display and connected to the input of a transistor.  Turning on the GPIO pin causes current to sink through the LED segment into the open collector output of the transistor.

[![](http://img.youtube.com/vi/BE-GWVR0DsE/1.jpg)](https://youtu.be/BE-GWVR0DsE)<br>
[Watch the video](https://youtu.be/BE-GWVR0DsE)

## Experiment6-UART-NVIC
This experiment is a continuation of the previous experiment that tests out the UART on the Tiva board.   Instead of using an RS232 to TTL converter cable, it uses a Maxim 3232 transceiver chip with external tantalum (0.1uF) capacitors.

The code now uses an interrupt handler instead of polling the UART for received characters.  It has also been refactored to use a state machine, and the hardware functions were improved to eliminate redundancy and create a better level of abstraction for the calling program(s).

[![](http://img.youtube.com/vi/Sd4p9AEHWPo/2.jpg)](https://youtu.be/Sd4p9AEHWPo)<br>
[Watch the video](https://youtu.be/Sd4p9AEHWPo)

## Experiment5-UART
This experiment sets up UART5 on the Tiva board and connects to the PC using an FTDI RS-232 to 3.3v converter cable. The embedded application transmits a selection menu to the PC that allows the user to choose one of eight colors to display on the tri-color LED. PuTTY is used as the terminal program on the PC. 

A scope probe was hooked up to the TX pin (PE5) to view the waveform of the characters transmitted from the board. To see a more regular, periodic signal, the embedded app transmits the letter 'U' repeatedly when SW1 is held down and a menu selection is made. This appears on the oscilloscope as a series of alternating pulses.

[![](http://img.youtube.com/vi/AObu1TXMY2k/1.jpg)](https://youtu.be/AObu1TXMY2k)<br>
[Watch the video](https://youtu.be/AObu1TXMY2k)

## Experiment4-SysTick
This experiment tests the SysTick timer and PLL on the EV-TM4C123GLX board.  The PLL is enabled and configured to run the bus clock at 80MHz.  The preset for the SysTick timer is taken from the value of 8 GPIO inputs that are connected to an 8-pin DIP switch.  The binary value on the DIP switches is converted to decimal and multiplied by 10ms.  The maximum timer delay is therefore 255 * 10ms = 2.55 seconds.  

Three LEDs are connected to three GPIO output ports and turned on/off after the SysTick timer counts down to zero and rolls over.  The LEDs turn on/off in a shifting pattern from right to left.  The red, onboard LED is also made to blink at the same rate.  

[![](http://img.youtube.com/vi/_GmFn_izdOk/1.jpg)](https://youtu.be/_GmFn_izdOk)<br>
[Watch the video](https://youtu.be/_GmFn_izdOk)

## Experiment3-GPIO-Relay
This experiment tests the GPIO on the EV-TM4C123GLX board.  The purpose of it is to press a button that turns on an LED and accuate a relay that turns on a 12V fan motor.  The fan's power supply is different from the Tiva board's.

[![](http://img.youtube.com/vi/kNR2bn99Wfo/1.jpg)](https://youtu.be/kNR2bn99Wfo)<br>
[Watch the video](https://youtu.be/kNR2bn99Wfo)

## Experiment2-GPIO-RegisterAccess
This experiment tests the GPIO on the EV-TM4C123GLX board.  It is the same as Experiment1 except that it uses direct register access instead of the The TivaWare Peripheral Driver Library.

[![](http://img.youtube.com/vi/_HHdLuPHRIA/1.jpg)](https://youtu.be/_HHdLuPHRIA)<br>
[Watch the video](https://youtu.be/_HHdLuPHRIA)

## Experiment1-GPIO
This experiment tests the GPIO on the EV-TM4C123GLX board.  The code alternates turning on the onboard red/blue/green LEDs and uses the two switch inputs as an override to turn all LEDs on or off.  The TivaWare Peripheral Driver Library is being used.

[![](http://img.youtube.com/vi/_HHdLuPHRIA/1.jpg)](https://youtu.be/_HHdLuPHRIA)<br>
[Watch the video](https://youtu.be/_HHdLuPHRIA)