// File:  project.c
// Author: JS
// Date:  12/02/17
// Purpose: Flash Experiment
// Hardware:  TM4C123 Tiva board


#include "HAL.h"
#include "Strings.h"

typedef struct {
	uint32_t		red;
	uint32_t		blue;
	uint32_t		green;
	char 				name[16];
	// Structure size MUST be a multiple of 32-bit words if saved directly to Flash!  This one is.
} Color_t;


void InitHardware()
{
	PLL_Init80MHz();
	
	// Enable the onboard LEDs and Switches.
	GPIO_EnableDI(PORTF, PIN0 | PIN4, PULL_UP);
	GPIO_EnableDO(PORTF, PIN1 | PIN2 | PIN3, DRIVE_2MA, PULL_DOWN);
	
	SysTick_Init();
	
	Flash_Enable();
}


// Turns on/off the LEDs based on the user's color selection.
void SetLEDColor(uint8_t colorIndex)
{
	// PF1 == RED
	PF1 = (colorIndex & 0x4) >> 2;
	
	// PF2 == BLUE
	PF2 = (colorIndex & 0x1);
	
	// PF3 == GREEN
	PF3 = (colorIndex & 0x2) >> 1;
	
}


uint8_t LoadColor(void)
{
	
	Color_t color;
	uint8_t colorIndex = 0;
		
	// Read the stored color value from Flash and set to zero if it is invalid.
	Flash_Read(&color, sizeof(Color_t) / sizeof(uint32_t));
	
	if (color.red) {
		colorIndex |= 0x4;
	}
	
	if (color.green) {
		colorIndex |= 0x2;
	}
	
	if (color.blue) {
		colorIndex |= 0x1;
	}

	return colorIndex;
	
}


void SaveColor(uint8_t colorIndex)
{
	Color_t color;
	
	color.red = (colorIndex & 0x4) >> 2;
	color.green = (colorIndex & 0x2) >> 1;	
	color.blue = (colorIndex & 0x1);
	
	char* name;
	
	// Save a textual name also to verify writing/reading is working correctly.
	switch (colorIndex) {
		case 1:
			name = "BLUE";
			break;
		case 2:
			name = "GREEN";
			break;
		case 3:
			name = "CYAN";
			break;
		case 4:
			name = "RED";
			break;
		case 5:
			name = "MAGENTA";
			break;
		case 6:
			name = "YELLOW";
			break;
		case 7:
			name = "WHITE";
			break;
		case 0:
		default:
			name = "BLACK";
			break;
	}
	
	strncpy(color.name, name, sizeof(color.name));
	
	Flash_Write(&color, sizeof(Color_t) / sizeof(uint32_t));
	
}



int main()
{
	
	int isSwitch1Up = 0;
	int isSwitch2Up = 0;	
	uint8_t colorIndex = 0;
	
	InitHardware();
	
	// Load the saved color from Flash.
	colorIndex = LoadColor();
	
	// Set the initial LED color (loaded from Flash).
	SetLEDColor(colorIndex);
	
	while (1) 
	{
		
			// If Switch 1 is up, then set its Up flag.  If it is down, change the LED color only
			// 	if Switch 1 was previously up.  In this way, the LED will be changed only on the
			//	transition from up to down.
			if (PF4) {
				isSwitch1Up = 1;
			}
			else if (isSwitch1Up) {
				
				// Change to the next of eight colors in the cycle.
				colorIndex = (colorIndex + 1) % 8;
				
				// Display the color on the LEDs.
				SetLEDColor(colorIndex);
				
				// The switch is now down.
				isSwitch1Up = 0;
			}
			
		
			// If Switch 2 is up, then set its Up flag.  If it is down, then save the color only
			// 	if Switch 2 was previously up.  In this way, the save will occur only on the
			//	transition from up to down.
			if (PF0) {
				isSwitch2Up = 1;
			}
			else if (isSwitch2Up) {
					
				// Write the color into Flash memory.
				SaveColor(colorIndex);
				
				isSwitch2Up = 0;
			}
				
		
			// Wait 100ms...
			SysTick_Wait10ms(10);
	}
	
}
