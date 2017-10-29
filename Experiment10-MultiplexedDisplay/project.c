// File:  project.c
// Author: JS
// Date:  10/4/17
// Purpose: Multiplexed Display
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Utilities.h"
#include "Display.h"

static volatile uint32_t* counterMode;

void InitHardware()
{
	__disable_irq();
	
	PLL_Init80MHz();

	GPIO_InitPort(PORTA);
	GPIO_EnableDI(PORTA, PIN_2, PULL_DOWN);
	
	GPIO_InitPort(PORTB);
	GPIO_EnableDO(PORTB, PIN_2 | PIN_3 | PIN_6 | PIN_7, DRIVE_2MA, PULL_DOWN);
	
	GPIO_InitPort(PORTC);
	GPIO_EnableDO(PORTC, PIN_4 | PIN_5 | PIN_6 | PIN_7, DRIVE_2MA, PULL_UP);
		
	// These GPIO ports will be used for the display...
	Display_IOPorts_t displayPorts;
	displayPorts.bcdA = &PC4;
	displayPorts.bcdB = &PC5;
	displayPorts.bcdC = &PC6;
	displayPorts.bcdD = &PC7;
	displayPorts.digit0Enable = &PB2;
	displayPorts.digit1Enable = &PB3;
	displayPorts.digit2Enable = &PB6;
	displayPorts.digit3Enable = &PB7;
	Display_Initialize(&displayPorts);
	
	// PE3=ADC
	ADC_Enable(ADC0, AIN0);
	
	SysTick_Init();
	
	// This will be the counter mode button port.
	counterMode = &PA2;
	
	__enable_irq();
	
}


int main()
{
	uint32_t counter = 0;
	uint32_t displayValue = 0;
	
	InitHardware();

	while (1) 
	{			
		
		// If the counter mode button is pressed then display the counter value.
		//	Otherwise, display the ADC value.
		if (*counterMode) {
			displayValue = counter % 10000;
		}
		else {
			// Sample the ADC.
			displayValue = ADC_Sample(ADC0);
		}			
			
		// Display the value.
		Display_Update(displayValue);
		
		// Wait one second...
		SysTick_Wait10ms(100);

		// Update the counter.
		counter++;
		
	}
	
}
