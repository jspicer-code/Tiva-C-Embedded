#include "project.h"
#include <stdio.h>
#include <stdint.h>

void InitHardware()
{
	PLL_Init80MHz();
	
	GPIO_PortA_Init();
	GPIO_PortA_EnableDI(PIN_5 | PIN_6 | PIN_7, PULL_DOWN);
	GPIO_PortA_EnableDO(PIN_2 | PIN_3 | PIN_4, DRIVE_2MA);
	
	
	GPIO_PortE_Init();
	GPIO_PortE_EnableDI(PIN_1 | PIN_2 | PIN_3 | PIN_4 | PIN_5, PULL_DOWN);
	
	GPIO_PortF_Init();
	GPIO_PortF_EnableDO(PIN_1, DRIVE_2MA);
	
	SysTick_Init();
}


int main()
{
	uint32_t testLED = 0;
	uint8_t input = 0;
	uint8_t output = 1;
	
	InitHardware();
	
	while (true) 
	{
		
		input = PE1 | (PE2 << 1) | (PE3 << 2) | (PE4 << 3) | (PE5 << 4) | (PA5 << 5) | (PA6 << 6) | (PA7 << 7);
		
		SysTick_Wait10ms(input);
	
		output <<= 1;
		if (output > 0x4) {
			output = 0x1;
		}			
		
		PA2 = output & 0x1;
		PA3 = (output & 0x2) >> 1;
		PA4 = (output & 0x4) >> 2;
				
		testLED ^= 1;
		PF1 = testLED;
		
	}
	
}
	
