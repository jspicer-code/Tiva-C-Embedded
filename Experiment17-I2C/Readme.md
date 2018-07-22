# Experiment17-I2C

## Overview
This experiment connects three Tiva C boards together to form an I2C bus.  One of the boards is an EK-TM4C1294XL and the other two are EK-TM4C123GXLs.  The purpose of the experiment was to get familiar with I2C and I2C programming on the Tivas, and also to learn about the differences between the TM4C1294 and TM4C123.  

Each board is configured to be both a master and slave on the bus, and is connected to eight switches, one tri-color LED (or three separate color LEDs for the TM4C1294), and four green LEDs to display a number pattern. Three different commands can be sent from any one board to another by setting a target slave ID on the switches and pressing one or more of the onboard buttons.  Pressing SW1 invokes the WRITE command, which sends the switch settings from the master board to a slave board, and the slave will display the settings on its LEDs.  Pressing SW2 invokes the READ command, causing the master board to read the switch settings from the slave board and display them on its own LEDs.  Pressing both SW1 and SW2 invokes the BLINKY command, causing the slave board to enter into a blinking pattern.  The WRITE and BLINKY commands can also be sent to all boards on the bus in one transaction that uses repeated starts.

The Keil project has separate build targets for the TM4C1294 and TM4C123, and there are startup folders for each board that contain a different startup file and main function.  Building the project for one target disables the startup files for the other.  In addition, the HAL modules use conditional compilation when there are implementation difference between the two processors.  Flags in the HAL_Config.h file will turn on/off these conditional features.
 
![Experiment17-I2C](Experiment17-I2C-circuit.png)  