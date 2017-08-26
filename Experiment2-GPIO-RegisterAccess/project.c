#include <stdio.h>
#include <stdint.h>
#include "project.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


int main(void)
{
    volatile uint32_t ui32Loop;
	  uint8_t input = 0;
	  uint8_t isSW1Down = 0;
	  uint8_t isSW2Down = 0;
	  int ledNumber = 0;
	  int outputPin;
		
		// Enable and provide a clock to GPIO PortF in Run mode
		SYSCTL_RCGCGPIO_R |= 0x20;
	
		// Wait until PORTF becomes present.
		while ((SYSCTL_PRGPIO_R & 0x20) == 0) {}
			
		// PIN_0 is normally locked.  Need to write a special passcode and commit it.
		GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
		GPIO_PORTF_CR_R = 0x1;
   
		// Disable analog function
		GPIO_PORTF_AMSEL_R = 0x00;
			
		// Set digital function to regular I/O, no special functions.	
		GPIO_PORTF_PCTL_R = 0x00000000;	
			
		// Set PF0 and PF4 for input, and the rest for output.	
		GPIO_PORTF_DIR_R = 0xEE;	
			
		// Disable alternate functions.
		GPIO_PORTF_AFSEL_R = 0x00;		
			
		// Enable digital I/O
		GPIO_PORTF_DEN_R = 0xFF;	
    
		// Enable weak pullup for PF0 and PF4 inputs.	
		GPIO_PORTF_PUR_R = 0x11;	
			
		while(1)
    {
			// Read the inputs...
			input = GPIO_PORTF_DATA_R;
			isSW1Down = ~(input >> 4) & 0x01;
			isSW2Down = ~(input) & 0x01;
			
			if (isSW1Down) {
				// Turn on all LEDs connected to PF3-PF1.  Color will turn WHITE.
				GPIO_PORTF_DATA_R = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
			}
			else if (isSW2Down) {
				// Turn off all LEDs connected to PF3-PF1.
				GPIO_PORTF_DATA_R = 0x00;
			}
			else {
				
				switch (ledNumber) {
					case 0:
						outputPin = GPIO_PIN_1;
						break;
					case 1:
						outputPin = GPIO_PIN_2;
						break;
					default:
						outputPin = GPIO_PIN_3;
						break;
				}
				
				// Turn on the selected LED.
				GPIO_PORTF_DATA_R = outputPin;

				// Delay for a bit.
				for(ui32Loop = 0; ui32Loop < 400000; ui32Loop++)
				{
				}

				// Turn off the selected LED.
				GPIO_PORTF_DATA_R = 0x00;
				
				// Alternate RED, BLUE, GREEN, RED, ....
				ledNumber = (ledNumber + 1) % 3;
			
			}     
    }
}
