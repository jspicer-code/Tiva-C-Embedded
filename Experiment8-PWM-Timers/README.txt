Source Code Overview  

This text is taking from the TivaWare™ Peripheral Driver Library pdf but is put here for quick
reference.

The following is an overview of the organization of the peripheral driver library source code.
EULA.txt The full text of the End User License Agreement that covers the use of this
software package.

**** Folder Structure

driverlib/ 		This directory contains the source code for the drivers.

hw_*.h 			Header files, one per peripheral, that describe all the registers and the bit
				fields within those registers for each peripheral. These header files are used
				by the drivers to directly access a peripheral, and can be used by application
				code to bypass the peripheral driver library API.

inc/ 			This directory holds the part specific header files used for the direct register
				access programming model.
				
makedefs 		A set of definitions used by make files.

Libraries/		The folder for precompiled libraries

src/ 			Place to put user code


**** Programming model

The peripheral driver library provides support for two programming models. Each model can be 
used independently or combined, based on the needs of the application or the programming 
environment desired by the developer. Refer to the TivaWare™ Peripheral Driver Library pdf
for more information on each.


Direct register access:

	In the direct register access model, the peripherals are programmed by the application by 
	writing	values directly into the peripheral’s registers by using bitfields or constant 
	values.  
	
	Example: SSI0_CR0_R = ((5 << SSI_CR0_SCR_S) | SSI_CR0_SPH | SSI_CR0_SPO);
			 SSI0_CR0_R =  0x00002231;

	The header file for the TM4C123GH6PM  microcontroller is "inc/tm4c123gh6pm.h" and must
	be included for this method to work.


Model and the software driver model.

	In the software driver model, the API provided by the peripheral driver library is used 
	by applications to control the peripherals. Because these drivers provide complete 
	control of the peripherals in their normal mode of operation, it is possible to write 
	an entire application without direct access to the hardware. 
	
	Example: SSIConfigSetExpClk(SSI0_BASE, 50000000, SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER, 1000000, 8);
	
	This project is currently configured so you may start coding with this method.