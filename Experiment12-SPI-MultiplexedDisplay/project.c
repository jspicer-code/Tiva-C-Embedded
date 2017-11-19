// File:  project.c
// Author: JS
// Date:  10/4/17
// Purpose: MuxDisplay + SPI experiment
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Display.h"

volatile uint32_t* SW1;
volatile uint32_t* SW2;


void InitHardware()
{
	__disable_irq();
	
	PLL_Init80MHz();

	GPIO_InitPort(PORTA);
	GPIO_EnableDI(PORTA, PIN_6 | PIN_7, PULL_UP);
	
	// PE3 = Right Pot
	ADC_Enable(ADC0, AIN0);
	
	// PE2 = Left Pot
	ADC_Enable(ADC1, AIN1);
	
	Display_Initialize();
	
	SysTick_Init();
	
	// Store the switch addresses.
	SW1 = &PA6;
	SW2 = &PA7;
	
	__enable_irq();
	
}


int main()
{

	uint32_t sample0;
	uint32_t sample1;
	
	InitHardware();
	
	while (1) 
	{			
		
		// Sample, then trim off LSBs and scale into range to avoid jitter.	
		sample0 = ADC_Sample(ADC0);		
		sample0 &= 0xFFFFFFF0;
		sample0 = (float)sample0 * 1.00368f;
		
		// Sample, then trim off LSBs and scale into range to avoid jitter.	
		sample1 = ADC_Sample(ADC1);
		sample1 &= 0xFFFFFFF0;
		sample1 = (float)sample1 * 1.00368f;

		
		if (!(*SW1)) {
		
			Display_Update(sample1);
			
		}
		else if (!(*SW2)) {
			
			Display_Update(sample0);
			
		}
		else {
			
			Display_Blank();

		}
		
		// Wait 100ms...
		SysTick_Wait10ms(10);

	}
	
}
