// File:  HAL_GPIO.c
// Author: JSpicer
// Date:  9/9/17
// Purpose: GPIO utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "HAL.h"

typedef struct {
	uint32_t DATA_BITS[255];
	uint32_t DATA;
	uint32_t DIR;
	uint32_t IS;
	uint32_t IBE;
	uint32_t IEV;
	uint32_t IM;
	uint32_t RIS;
	uint32_t MIS;
	uint32_t ICR;
	uint32_t AFSEL;
	uint32_t RESERVED[55];
	uint32_t DR2R;
	uint32_t DR4R;
	uint32_t DR8R;
	uint32_t ODR;
	uint32_t PUR;
	uint32_t PDR;
	uint32_t SLR;
	uint32_t DEN;
	uint32_t LOCK;
	uint32_t CR;
	uint32_t AMSEL;
	uint32_t PCTL;
	uint32_t ADCCTL;
	uint32_t DMACTL;
} GPIORegs_t;


// A lookup up table for base addresses of the ports
//	Use a PortName_t value for an index.
const volatile uint32_t * GPIOBaseAddress[] = {
	GPIO_PORTA_DATA_BITS_R,
	GPIO_PORTB_DATA_BITS_R,
	GPIO_PORTC_DATA_BITS_R,
	GPIO_PORTD_DATA_BITS_R,
	GPIO_PORTE_DATA_BITS_R,
	GPIO_PORTF_DATA_BITS_R
};

static int GetPinNumber(PinDef_t pinDef) 
{
	for (int i = 0; i < 8; i++) {
			if ((pinDef.pin >> i) & 0x1) {
				return i;
			}			
	}
	return 0;
}

volatile uint32_t* GPIO_GetBitBandIOAddress(PinDef_t pinDef)
{	
	const volatile uint32_t * baseAddress = GPIOBaseAddress[pinDef.port];
	
	int pinNumber = GetPinNumber(pinDef);
	
	// Add 0x3FC (data register offset) and clear the top byte to get the data address.
	uint32_t dataAddress = 0x00FFFFFF & ((uint32_t)baseAddress + 0x3FC);
	
	return (volatile uint32_t *)(0x42000000 + (32*dataAddress) + (4*pinNumber));
}



void GPIO_InitPort(PortName_t port)
{
	
	uint32_t portBit = (0x1 << (uint8_t)port);
	
	if (!(SYSCTL_PRGPIO_R & portBit)) {
	
		// Clock gating control register - enables the GPIO function for the port.
		SYSCTL_RCGCGPIO_R |= portBit; 
	
		// Perpheral ready register - wait until the correponding port bit is set (enabled).
		while (!(SYSCTL_PRGPIO_R & portBit )){}
	}
}


void GPIO_EnableDO(PortName_t port, uint8_t pinMap, DO_Drive_t drive, DI_Pull_t pull)
{
		
	GPIO_InitPort(port);
		
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	
	if (port == PORTF && pinMap & 0x01) {
		// PORTF PIN_0 is normally locked.  Need to write a special passcode and commit it.
		gpio->LOCK = GPIO_LOCK_KEY;
		gpio->CR = 0x01;
	}

	// Set pins for output
	gpio->DIR |= pinMap;
	
	// Disable alternate functions
	gpio->AFSEL &= ~pinMap;	
	
	// Set the output drive strength.
	switch (drive) {
		case DRIVE_2MA:
			gpio->DR2R |= pinMap;
			break;
		
		case DRIVE_4MA:
			gpio->DR4R |= pinMap;
			break;
		
		case DRIVE_8MA:
			gpio->DR8R |= pinMap;
			break;
	};
	
	// Set the pull up/down mode.
	switch (pull) {
		case PULL_NONE:
			gpio->PUR &= ~pinMap;	
		  gpio->PDR &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			gpio->PUR |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			gpio->PDR |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
}

void GPIO_EnableDI(PortName_t port, uint8_t pinMap, DI_Pull_t pull)
{
	
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	
	if (port == PORTF && pinMap & 0x01)  {
		// PORTF PIN_0 is normally locked.  Need to write a special passcode and commit it.
		gpio->LOCK = GPIO_LOCK_KEY;
		gpio->CR = 0x01;
	}
	
	// Clear pins for input
	gpio->DIR &= ~pinMap;
	
	// Disable alternate functions
	gpio->AFSEL &= ~pinMap;	
	
	// Set the pull up/down mode.
	switch (pull) {
		case PULL_NONE:
			gpio->PUR &= ~pinMap;	
		  gpio->PDR &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			gpio->PUR |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			gpio->PDR |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
}

void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	uint32_t pctlValue = 0;
	int i;
	
	// Enable pins for digital and disable for analog.
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
	// Enable pins for alternate function.
	gpio->AFSEL |= pinMap;
	
	for (i = 0; i < 8; i++) {
		if (pinMap & (0x1 << i)) {
			pctlValue |= (ctl << (4*i));
		}
	}
	
	// Set the pin control function for each pin.
	gpio->PCTL |= pctlValue;

}


void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];

	// Enable pins for analog and disable for digital.
	gpio->DEN &= ~pinMap;
	gpio->AMSEL |= pinMap;
	
	// Not seemingly necessary, but disable pull ups/downs anyway.
	gpio->PUR &= ~pinMap;	
	gpio->PDR &= ~pinMap;	
	
	// Enable pins for alternate function.
	gpio->AFSEL |= pinMap;
}
