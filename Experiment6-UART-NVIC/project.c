// File:  project.c
// Author: JS
// Date:  9/9/17
// Purpose: UART Experiment
// Hardware:  TM4C123 Tiva board

#include "project.h"
#include <stdio.h>
#include <stdint.h>


enum {
	STATE_MENU,
	STATE_SELECTION,
	STATE_LEDS,
	STATE_TEST_PATTERN
} AppState;

uint8_t LEDColor;

void RxCallback(char c)
{
	switch (AppState) {
		
		case STATE_SELECTION:
			
			if (c >= '1' && c <= '8') {
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
	// The selection entry is already an RGB bit pattern.  Just need to 
	// 	subtract one to make it zero based.
	//uint8_t rgbColor = LEDColor; //selection - 1;
	
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
	// Wait 100 ms.
	SysTick_Wait10ms(10);

	// An upper-case ASCII 'U' results in square wave.
	UART_WriteChar(UART5, 'U');
}

// Main function.  Initializes the hardware then loops doing the following:
// 1. Send the selection menu
// 2. Wait for a selection character to be pressed.
// 3. If the onboard SW1 is held down, transmit a test pattern until SW1 is released.
// 4. If the user's selection is valid, turn on the corresponding LED color.  Otherwise, ignore the selection.
int main()
{
	
	InitHardware();
	
	while (true) 
	{
		
		switch (AppState)
		{
			case STATE_MENU:
				PrintMenu();
				AppState = STATE_SELECTION;
				break;
		
			case STATE_SELECTION:
				// do nothing
				break;
			
			case STATE_LEDS:
				SetLEDColor(LEDColor);
				AppState = STATE_MENU;
				break;
			
			case STATE_TEST_PATTERN:
				SendTestPattern();
				break;
		}	
		
	}
	
}

	
