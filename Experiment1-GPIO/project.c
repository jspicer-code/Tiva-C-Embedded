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
		
    // Enable the GPIO port that is used for the on-board LED.
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
		// Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {    
		}
    
		// Enable the GPIO pin for the LED (PF3-PF1).  Set the direction as output, and
    // enable the GPIO pin for digital function.
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);
		
		// PIN_0 is normally locked.  Need to write a special passcode and commit it.
		GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
		GPIO_PORTF_CR_R = 0x1;
		
		// Enable the GPIO pin for the switches (PF0 and PF4).  Set the direction as input, and
    // enable the GPIO pin for digital function.
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    
		// Enabled the weak pullup for the input pins.
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
				
		while(1)
    {
			// Read the inputs...
			input = (uint8_t)GPIOPinRead(GPIO_PORTF_BASE,	GPIO_PIN_0 | GPIO_PIN_4); // & 0x11);
			isSW1Down = ~(input >> 4) & 0x01;
			isSW2Down = ~(input) & 0x01;
			
			if (isSW1Down) {
				// Turn on all LEDs.  Color will turn WHITE.
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			}
			else if (isSW2Down) {
				// Turn off all LEDs.
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x0);
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
				GPIOPinWrite(GPIO_PORTF_BASE, outputPin, outputPin);

				// Delay for a bit.
				for(ui32Loop = 0; ui32Loop < 400000; ui32Loop++)
				{
				}

				// Turn off the selected LED.
				GPIOPinWrite(GPIO_PORTF_BASE, outputPin, 0x0);
				
				// Alternate RED, BLUE, GREEN, RED, ....
				ledNumber = (ledNumber + 1) % 3;
			
			}     
    }
}
