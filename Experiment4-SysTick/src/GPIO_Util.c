#include <stdint.h>
#include "GPIO_Util.h"
#include "tm4c123gh6pm.h"



void GPIO_PortA_Init(void)
{

	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
	
	while (!(SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0)){}
	
	// NOTE: Don't change the pin configuration for pins 0 and 1!
	// 	They are being used by the DEBUG USB UART.	
}


	
void GPIO_PortA_EnableDO(uint8_t pinMap, DO_Drive_t drive)
{
	// Set pins for output
	GPIO_PORTA_DIR_R |= pinMap;
	
	// Disable alternate functions
	GPIO_PORTA_AFSEL_R &= ~pinMap;	
	
	// Set the output drive strength.
	switch (drive) {
		case DRIVE_2MA:
			GPIO_PORTA_DR2R_R |= pinMap;
			break;
		
		case DRIVE_4MA:
			GPIO_PORTA_DR4R_R |= pinMap;
			break;
		
		case DRIVE_8MA:
			GPIO_PORTA_DR8R_R |= pinMap;
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	GPIO_PORTA_DEN_R |= pinMap;
	GPIO_PORTA_AMSEL_R &= ~pinMap;
}
	
void GPIO_PortA_EnableDI(uint8_t pinMap, DI_Pull_t pull)
{
	// Clear pins for input
	GPIO_PORTA_DIR_R &= ~pinMap;
	
	// Disable alternate functions
	GPIO_PORTA_AFSEL_R &= ~pinMap;	
	
	// Set the output drive strength.
	switch (pull) {
		case PULL_NONE:
			GPIO_PORTA_PUR_R &= ~pinMap;	
		  GPIO_PORTA_PDR_R &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			GPIO_PORTA_PUR_R |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			GPIO_PORTA_PDR_R |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	GPIO_PORTA_DEN_R |= pinMap;
	GPIO_PORTA_AMSEL_R &= ~pinMap;
	
}



void GPIO_PortE_Init(void)
{

	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4 ;
	
	while (!(SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R4 )){}
	
}


void GPIO_PortE_EnableDO(uint8_t pinMap, DO_Drive_t drive)
{
	// Set pins for output
	GPIO_PORTE_DIR_R |= pinMap;
	
	// Disable alternate functions
	GPIO_PORTE_AFSEL_R &= ~pinMap;	
	
	// Set the output drive strength.
	switch (drive) {
		case DRIVE_2MA:
			GPIO_PORTE_DR2R_R |= pinMap;
			break;
		
		case DRIVE_4MA:
			GPIO_PORTE_DR4R_R |= pinMap;
			break;
		
		case DRIVE_8MA:
			GPIO_PORTE_DR8R_R |= pinMap;
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	GPIO_PORTE_DEN_R |= pinMap;
	GPIO_PORTE_AMSEL_R &= ~pinMap;
}

void GPIO_PortE_EnableDI(uint8_t pinMap, DI_Pull_t pull)
{
	// Clear pins for input
	GPIO_PORTE_DIR_R &= ~pinMap;
	
	// Disable alternate functions
	GPIO_PORTE_AFSEL_R &= ~pinMap;	
	
	// Set the output drive strength.
	switch (pull) {
		case PULL_NONE:
			GPIO_PORTE_PUR_R &= ~pinMap;	
		  GPIO_PORTE_PDR_R &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			GPIO_PORTE_PUR_R |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			GPIO_PORTE_PDR_R |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	GPIO_PORTE_DEN_R |= pinMap;
	GPIO_PORTE_AMSEL_R &= ~pinMap;
	
}


void GPIO_PortF_Init(void)
{
	// Enable and provide a clock to GPIO PortF in Run mode
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;

	// Wait until PORTF becomes present.
	while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0) {}
		
}


void GPIO_PortF_EnableDO(uint8_t pinMap, DO_Drive_t drive)
{
	
	if (pinMap & 0x01) 
	{
		// PIN_0 is normally locked.  Need to write a special passcode and commit it.
		GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
		GPIO_PORTF_CR_R = 0x01;
	}
	
	// Set pins for output
	GPIO_PORTF_DIR_R |= pinMap;
	
	// Disable alternate functions
	GPIO_PORTF_AFSEL_R &= ~pinMap;	
	
	// Set the output drive strength.
	switch (drive) {
		case DRIVE_2MA:
			GPIO_PORTF_DR2R_R |= pinMap;
			break;
		
		case DRIVE_4MA:
			GPIO_PORTF_DR4R_R |= pinMap;
			break;
		
		case DRIVE_8MA:
			GPIO_PORTF_DR8R_R |= pinMap;
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	GPIO_PORTF_DEN_R |= pinMap;
	GPIO_PORTF_AMSEL_R &= ~pinMap;
}


void GPIO_PortF_EnableDI(uint8_t pinMap, DI_Pull_t pull)
{
	
	if (pinMap & 0x01) 
	{
		// PIN_0 is normally locked.  Need to write a special passcode and commit it.
		GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
		GPIO_PORTF_CR_R = 0x01;
	}
	
	// Clear pins for input
	GPIO_PORTF_DIR_R &= ~pinMap;
	
	// Disable alternate functions
	GPIO_PORTF_AFSEL_R &= ~pinMap;	
	
	// Set the output drive strength.
	switch (pull) {
		case PULL_NONE:
			GPIO_PORTF_PUR_R &= ~pinMap;	
		  GPIO_PORTF_PDR_R &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			GPIO_PORTF_PUR_R |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			GPIO_PORTF_PDR_R |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	GPIO_PORTF_DEN_R |= pinMap;
	GPIO_PORTF_AMSEL_R &= ~pinMap;
	
}
