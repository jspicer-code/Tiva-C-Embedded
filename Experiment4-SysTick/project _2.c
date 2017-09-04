#include "project.h"
#include <stdio.h>
#include <stdint.h>

#define PA2 (*((volatile uint32_t*)0x42087F88)) 
#define PA3 (*((volatile uint32_t*)0x42087F8C)) 
#define PA4 (*((volatile uint32_t*)0x42087F90)) 
#define PA5 (*((volatile uint32_t*)0x42087F94)) 
#define PA6 (*((volatile uint32_t*)0x42087F98)) 
#define PA7 (*((volatile uint32_t*)0x42087F9C)) 

#define PF0 (*((volatile uint32_t*)0x424A7F80))
#define PF1 (*((volatile uint32_t*)0x424A7F84))
#define PF2 (*((volatile uint32_t*)0x424A7F88))
#define PF3 (*((volatile uint32_t*)0x424A7F8C))
#define PF4 (*((volatile uint32_t*)0x424A7F90))


void PortA_Init(void)
{

	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
	
	while (!(SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0)){}
	
	// Don't change the pin configuration for pins 0 and 1!
	// 	They are being used by the DEBUG USB UART.	
		
  // Set pins 4 and 5 for output.		
	GPIO_PORTA_DIR_R |= 0x30;
	
	// Clear pins 7,6,3, and 2 for input.
	GPIO_PORTA_DIR_R &= ~0xCC;
		
	// Disable alternate functions for pins 7-2.	
	GPIO_PORTA_AFSEL_R &= ~0xFC;	
		
	// Set the output drive strength to 2mA for pins 4 & 5.
	GPIO_PORTA_DR2R_R |= 0x30;
		
	// Enable weak pull resistors for input pins 7,6,3 and 2.	
	// 	This will create a negative logic interface.
	GPIO_PORTA_PUR_R |= 0xCC;	
		
	// Enable pins 7-2 for digital I/O and clear for analog I/O.
	GPIO_PORTA_DEN_R |= 0xFC;
	GPIO_PORTA_AMSEL_R &= ~0xFC;
	
}

void PortF_Init(void)
{
	// Enable and provide a clock to GPIO PortF in Run mode
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;

	// Wait until PORTF becomes present.
	while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0) {}
		
	// PIN_0 is normally locked.  Need to write a special passcode and commit it.
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
	GPIO_PORTF_CR_R = 0x01;
 
	// Set pins 1-3 for output.		
	GPIO_PORTF_DIR_R |= 0x0E;

	// Clear pins 0 and 4 for input.
	GPIO_PORTF_DIR_R &= ~0x11;
		
	// Disable alternate functions for pins 4-0.	
	GPIO_PORTF_AFSEL_R &= ~0x1F;	
		
	// Enable weak pullup for PF0 and PF4 inputs.	
	GPIO_PORTF_PUR_R = 0x11;	
			
	// Enable digital I/O for pins 4-0.  Disable analog I/O.
	GPIO_PORTF_DEN_R |= 0x1F;	
	GPIO_PORTA_AMSEL_R &= ~0x1F;
    
}


int main()
{
	
	PortA_Init();
	PortF_Init();
	
	while (true) 
	{
	
		PA4 = PF4;	// Switch 1
		PA5 = PF0;	// Switch 2
	
		if (PA7) {
			PF1 = !PA2; // Red
			PF3 = !PA3; // Green
			PF2 = !PA6; // Blue
		}
		else {
			// All on (white) if PA7 low.
			PF3 = PF2 = PF1 = 1;
		}
		
	}
	
}
	
