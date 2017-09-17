// File:  project.c
// Author: JS
// Date:  9/9/17
// Purpose: UART Experiment w/Interrupt
// Hardware:  TM4C123 Tiva board

#include "project.h"
#include <stdio.h>
#include <stdint.h>


// Program states.
enum {
	STATE_MENU,
	STATE_SELECTION,
	STATE_LEDS,
	STATE_TEST_PATTERN
} AppState;


// Holds the current LED color
uint8_t LEDColor;

// This function is called back by the Receive (Rx) UART interrupt handler
//	when a new character has arrived on the serial port.  Its purpose is
//	to examine the character input and change states for the main() function
//	to process.
void RxCallback(char c)
{
	switch (AppState) {
		
		case STATE_SELECTION:
			
			if (c >= '1' && c <= '8') {
			
				// The selection entry is already an RGB bit pattern.  Just need to 
				// 	subtract one and mask out the upper bits.
				LEDColor = (c & 0x07) - 1;
				AppState = STATE_LEDS;
			
			}
			else if (c == 'u' || c == 'U') {
				AppState = STATE_TEST_PATTERN;
			}
			
			break;
	
		case STATE_TEST_PATTERN:
			
			if (c == 'u' || c == 'U') {
				AppState = STATE_MENU;
			}
			break;
		
	}
}


// Initializes the PLL, UART, SysTick, and ports needed for the program.
void InitHardware()
{
	__disable_irq();
	
	PLL_Init80MHz();

	SysTick_Init();

	GPIO_Init_Port(PORTF);
	GPIO_EnableDO(PORTF, PIN_1 | PIN_2 | PIN_3, DRIVE_2MA);
	GPIO_EnableDI(PORTF, PIN_4, PULL_UP);
	
	UART_Init(UART5, 9600);
	UART_EnableRxInterrupt(UART5, RxCallback);

	__enable_irq();
}



// Transmits the color selection menu over the UART.
void PrintMenu(void)
{
	
	UART_WriteString(UART5, "\n\r\n\r");		
	UART_WriteString(UART5, "Select an LED color:\n\r");
	UART_WriteString(UART5, "1) Black (OFF)\n\r");
	UART_WriteString(UART5, "2) Blue\n\r");
	UART_WriteString(UART5, "3) Green\n\r");
	UART_WriteString(UART5, "4) Cyan\n\r");
	UART_WriteString(UART5, "5) Red\n\r");
	UART_WriteString(UART5, "6) Magenta\n\r");
	UART_WriteString(UART5, "7) Yellow\n\r");
	UART_WriteString(UART5, "8) White\n\r");
	UART_WriteString(UART5, "U) Test Pattern (Press U again to Stop)\n\r");
	UART_WriteString(UART5, ">");	

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


// Transmits the character 'U' for scope testing
void SendTestPattern()
{
	// Wait 100 ms.
	SysTick_Wait10ms(10);

	// An upper-case ASCII 'U' results in square wave.
	UART_WriteChar(UART5, 'U');
}

// Main function.  Initializes the hardware then loops while state changes occur.
int main()
{
	
	InitHardware();
	
	while (true) 
	{
		
		switch (AppState)
		{
			// Print the selection menu and transition to the selection state.
			case STATE_MENU:
				PrintMenu();
				AppState = STATE_SELECTION;
				break;
		
			// Remain in this state until the user makes a menu selection.
			case STATE_SELECTION:
				// do nothing
				break;
			
			// The user has made a valid selection, so write to the LEDs and transition
			//	back to the menu state.		
			case STATE_LEDS:
				SetLEDColor(LEDColor);
				AppState = STATE_MENU;
				break;
			
			// The user has selected to write the test pattern, so remain in this state
			//	until they have selected to go back to the menu.
			case STATE_TEST_PATTERN:
				SendTestPattern();
				break;
		}	
		
	}
	
}

	
