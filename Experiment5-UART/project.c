// File:  project.c
// Author: JS
// Date:  9/9/17
// Purpose: UART Experiment
// Hardware:  TM4C123 Tiva board

#include "project.h"
#include <stdio.h>
#include <stdint.h>

// Initializes the PLL, UART, SysTick, and ports needed for the program.
void InitHardware()
{
	PLL_Init80MHz();

	GPIO_PortF_Init();
	GPIO_PortF_EnableDO(PIN_1 | PIN_2 | PIN_3, DRIVE_2MA);
	GPIO_PortF_EnableDI(PIN_4, PULL_UP);
	
	UART_UART5_Init();

	SysTick_Init();
}

// Transmits the color selection menu over the UART.
void PrintMenu(void)
{
	
	UART_UART5_WriteString("\n\r\n\r");		
	UART_UART5_WriteString("Select an LED color:\n\r");
	UART_UART5_WriteString("1) Black (OFF)\n\r");
	UART_UART5_WriteString("2) Blue\n\r");
	UART_UART5_WriteString("3) Green\n\r");
	UART_UART5_WriteString("4) Cyan\n\r");
	UART_UART5_WriteString("5) Red\n\r");
	UART_UART5_WriteString("6) Magenta\n\r");
	UART_UART5_WriteString("7) Yellow\n\r");
	UART_UART5_WriteString("8) White\n\r");
	UART_UART5_WriteString(">");	

}

// Turns on/off the LEDs based on the user's color selection.
void SetLEDColor(int selection)
{
	// The selection entry is already an RGB bit pattern.  Just need to 
	// 	subtract one to make it zero based.
	uint8_t rgbColor = selection - 1;
	
	// PF1 == RED
	PF1 = (rgbColor & 0x4) >> 2;
	
	// PF2 == BLUE
	PF2 = (rgbColor & 0x1);
	
	// PF3 == GREEN
	PF3 = (rgbColor & 0x2) >> 1;
	
}

// Returns true if the test switch (SW1) is down.
bool IsTestSwitchDown(void)
{
	return !PF4;
}

// Transmits the character 'U' every 100ms for scope testing
void SendTestPattern()
{
	while (IsTestSwitchDown()) {

		// Wait 100 ms.
		SysTick_Wait10ms(10);

		// An upper-case ASCII 'U' results in square wave.
		UART_UART5_WriteChar('U');

	}
}

// Main function.  Initializes the hardware then loops doing the following:
// 1. Send the selection menu
// 2. Wait for a selection character to be pressed.
// 3. If the onboard SW1 is held down, transmit a test pattern until SW1 is released.
// 4. If the user's selection is valid, turn on the corresponding LED color.  Otherwise, ignore the selection.
int main()
{
	char selection;
	
	InitHardware();
	
	while (true) 
	{
		
		PrintMenu();
		
		do 
		{
			selection = UART_UART5_ReadChar();
			
			if (IsTestSwitchDown())
			{
				SendTestPattern();
			}
			
		} 
		while (selection < '1' || selection > '8');
		
		SetLEDColor(selection);
		
	}
	
}

	
