// File:  project.c
// Author: JS
// Date:  12/02/17
// Purpose: Flash Experiment
// Hardware:  TM4C123 Tiva board


#include "HAL.h"


void InitHardware()
{
	PLL_Init80MHz();
	
	// Enable the onboard LEDs and Switch 1.
	GPIO_EnableDI(PORTF, PIN4, PULL_UP);
	GPIO_EnableDO(PORTF, PIN1 | PIN2 | PIN3, DRIVE_2MA, PULL_DOWN);
	
	SysTick_Init();
	
	Flash_Enable();
}


// Turns on/off the LEDs based on the user's color selection.
void SetLEDColor(uint8_t rgbColor)
{
	// PF1 == RED
	PF1 = (rgbColor & 0x4) >> 2;
	
	// PF2 == BLUE
	PF2 = (rgbColor & 0x1);
	
	// PF3 == GREEN
	PF3 = (rgbColor & 0x2) >> 1;
	
}


int main()
{
	
	int isSwitchUp = 0;
	uint32_t rgbColor = 0;
	
	InitHardware();
	
	// Read the stored color value from Flash and set to zero if it is invalid.
	Flash_Read(&rgbColor, 1);
	if (rgbColor >= 8) {
		rgbColor = 0;
	}
	
	// Set the initial LED color (loaded from Flash).
	SetLEDColor((uint8_t)rgbColor);
	
	while (1) 
	{
		
			// If Switch 1 is up, then set the Up flag.  If it is down, change the LED color only
			// 	if Switch 1 was previously up.  In this way, the LED will be changed only on the
			//	transition from up to down.
			if (PF4) {
				isSwitchUp = 1;
			}
			else if (isSwitchUp) {
				
				// Change to the next of eight colors in the cycle.
				rgbColor = (rgbColor + 1) % 8;
				
				// Write the color into Flash memory.
				Flash_Write(&rgbColor, 1);
				
				// Display the color on the LEDs.
				SetLEDColor((uint8_t)rgbColor);
				
				// The switch is now down.
				isSwitchUp = 0;
			}
		
			// Wait 100ms...
			SysTick_Wait10ms(10);
	}
	
}
