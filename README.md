# Tiva C Series ARM development

This repository contains experiments developed for the EK-TM4C123GXL and EK-TM4C1294XL ARM evaluation boards.

# Experiments

## [Experiment18-RTC (Real-time Clock)](./Experiment18-RTC)

This experiment uses a DS1307 real-time clock IC to manage a calendar and clock.  The running date and time are displayed on an LCD,  and a five-position switch is used to set them.  The Tiva C board communicates with the chip using a 100kbps I2C connection.  A 3V coin cell battery keeps the clock running when main power is removed.   A driver was written that has functions to initialize and enable the clock, and read and write data from it.   This experiment runs on both the TM4C123G and TM4C1294.

[![](https://i.ytimg.com/vi/A5657PIc_eg/default.jpg)](https://youtu.be/A5657PIc_eg)<br>
[Watch the video](https://youtu.be/A5657PIc_eg)

## [Experiment17-I2C](./Experiment17-I2C)
This experiment connects three Tiva C boards to an I2C bus.  One of the boards is an EK-TM4C1294XL and the other two are EK-TM4C123GXLs.  The purpose of the experiment was to get familiar with I2C and I2C programming on the Tivas, and also to learn about the differences between the TM4C1294 and TM4C123.  

Each board is configured to be both a master and slave on the bus, and is connected to eight switches, one tri-color LED (or three separate color LEDs for the TM4C1294), and four green LEDs to display a number pattern. Three different commands can be sent from any one board to another by setting a target slave ID on the switches and pressing one or more of the onboard buttons.  Pressing SW1 invokes the WRITE command, which sends the switch settings from the master board to a slave board, and the slave will display the settings on its LEDs.  Pressing SW2 invokes the READ command, causing the master board to read the switch settings from the slave board and display them on its own LEDs.  Pressing both SW1 and SW2 invokes the BLINKY command, causing the slave board to enter into a blinking pattern.  The WRITE and BLINKY commands can also be sent to all boards on the bus in one transaction that uses repeated starts.

The Keil project has separate build targets for the TM4C1294 and TM4C123, and there are startup folders for each board that contain a different startup file and main function.  Building the project for one target disables the startup files for the other.  In addition, the HAL modules use conditional compilation when there are implementation difference between the two processors.  Flags in the HAL_Config.h file will turn on/off these conditional features.
 
[![](https://img.youtube.com/vi/yLTBi-FEyhM/2.jpg)](https://youtu.be/yLTBi-FEyhM)<br>
[Watch the video](https://youtu.be/yLTBi-FEyhM)

## [Experiment16-PWM-Music](./Experiment16-PWM-Music)
This experiment uses multiple PWM channels to produce audio signals and play tunes from MIDI files. A MIDI file exporter program was written in Visual Studio using the midifile parsing library (http://midifile.sapp.org/). The exporter generates a .c file that is compiled into the Keil project for the Tiva C. Up to four MIDI tracks are exported. 

Each MIDI track is controlled at run time by a FreeRTOS task that schedules and plays the track's note events. Playing a note involves modulating one of the Tiva's PWM channels to a frequency equal to the note's audio frequency. 

The PWM outputs are combined and amplified by a circuit that consists of tri-state buffers (stage 1) and an NJM386 audio amplifier (stage 2). Each PWM output controls the enable pin of a tri-state buffer, such that when the PWM pulse is HIGH, the buffer's output voltage is added to the input signal of stage 2; and when the PWM pulse is LOW, the buffer enters a high impedance state and is effectively removed from the audio input signal.

[![](https://i.ytimg.com/vi/MY4hRxabXYo/default.jpg)](https://youtu.be/MY4hRxabXYo)<br>
[Watch the video](https://youtu.be/MY4hRxabXYo)

## Experiment15-FreeRTOS
This experiment tries out FreeRTOS on the TM4C123 board, as well as GPIO interrupts and playing musical sounds using PWM.  The goal was to create some periodic tasks and preempt them aperiodically, to see if they suffered any noticeable jitter.

The firmware application creates three tasks that blink red, green, and yellow LEDs at different periods, while another task plays a musical sequence on the speaker when one of the onboard switches is pressed .  The tasks controlling the LEDs all have the same (lowest) priority, whereas the task that plays the sounds runs at a higher priority, and this effectively preempts the other tasks each time a note is played.  An ISR, which runs on the falling edge of one of the switches, "gives" a binary semaphore that the high priority task "takes".  Hence, the ISR defers control to a task rather than do the processing itself.

The hardware circuit uses a ULN2003 Darlington array that acts as a low-side driver, sinking current for the LEDs and speaker.  A PWM output drives the speaker at different frequencies to create the musical notes.

[![](https://i.ytimg.com/vi/h58GzA-6aEo/3.jpg)](https://youtu.be/h58GzA-6aEo)<br>
[Watch the video](https://youtu.be/h58GzA-6aEo)

## Experiment14-LCD
This experiment uses the EK-TM4C123GXL to drive two LCD displays based on the Hitachi HD44780 controller spec.  One is a 16x2 display w/backlight (Techstar TS1620-21/B) and the other is a 16x1 reflective display (Lumex LCM-S01601DSR).  The TM4C123 is configured to use GPIO (digital) pins to transfer data to the displays using 4-bit data bus mode.

A two-way switch on the breadboard controls which display receives the input (cursor) focus.  Cursor movement and character entry is controlled by an additional 5-position switch, which has directions for up, down, left, right, and center.  The cursor can be moved around via the up, down, left, and right positions.  Pressing the 5-position switch vertically downward (center position) causes the cursor to blink, and a new character can be selected by moving the switch in the other directions:  left/right traverses the alphabet and symbols, whereas up/down toggles between upper and lower case and a different set of symbols.  Pressing the center switch down again locks in the character. 

Holding down the on-board switch, SW2, causes the both displays to side scroll together, like a single 32x2 marquee. 

The LCD driver is split into lower (raw) and higher level components.  The raw driver handles communication with the HD44780 controller, and implements eight or so of its instruction codes, such as Function Set, Set DDRAM Address, Entry Mode Set, etc.  The higher level LCD driver has more abstract functions for writing text and moving the cursor around, and handles the differences between a 16x1 and 16x2 display, such as selecting DDRAM addresses.

[![](https://i.ytimg.com/vi/5TJ8u1CpPAs/default.jpg)](https://youtu.be/5TJ8u1CpPAs)<br>
[Watch the video](https://youtu.be/5TJ8u1CpPAs)

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