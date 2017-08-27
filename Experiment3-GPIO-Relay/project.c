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
    uint8_t input = 0;
	  uint8_t isSW1Down = 0;
	 
		
		// Enable and provide a clock to GPIO PortE in Run mode
		SYSCTL_RCGCGPIO_R |= 0x10;
	
		// Wait until PORTE becomes present.
		while ((SYSCTL_PRGPIO_R & 0x10) == 0) {}
			
			// Disable analog function
		GPIO_PORTE_AMSEL_R = 0x00;
			
		// Set digital function to regular I/O, no special functions.	
		GPIO_PORTE_PCTL_R = 0x00000000;	
			
		// Set PE1 for input, PE2 and PE3 for output.	
		GPIO_PORTE_DIR_R = 0x0C;	
			
		// Disable alternate functions.
		GPIO_PORTE_AFSEL_R = 0x00;		
			
		// Enable digital I/O
		GPIO_PORTE_DEN_R = 0x0E;	
					
		// Disable weak pullups	& pulldowns
		GPIO_PORTE_PUR_R = 0x00;	
		GPIO_PORTE_PDR_R = 0x00;	
		
		// Enable 2ma drive strength.
		GPIO_PORTE_DR2R_R = 0x0C;	
			
			
		while(1)
    {
			// Read the inputs...
			input = GPIO_PORTE_DATA_R;
			isSW1Down = (input >> 1) & 0x01;
			
			// If the switch is down, turn on the outputs.  Otherwise, turn them off.
			if (isSW1Down) {
				GPIO_PORTE_DATA_R = GPIO_PIN_2 | GPIO_PIN_3;
			}
			else {
				GPIO_PORTE_DATA_R = 0x00;
			}
			

    }
}
