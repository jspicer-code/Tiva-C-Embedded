// File:  project.c
// Author: JS
// Date:  9/9/17
// Purpose: ADC Experiment
// Hardware:  TM4C123 Tiva board

#include "project.h"
#include <stdio.h>
#include <stdint.h>



// Initializes the PLL, UART, SysTick, and ports needed for the program.
void InitHardware()
{
//	__disable_irq();
//	
//	PLL_Init80MHz();

//	SysTick_Init();

//	GPIO_Init_Port(PORTF);
//	GPIO_EnableDO(PORTF, PIN_1 | PIN_2 | PIN_3, DRIVE_2MA);
//	GPIO_EnableDI(PORTF, PIN_4, PULL_UP);
//	
//	UART_Init(UART5, 9600);
//	UART_EnableRxInterrupt(UART5, RxCallback);

//	__enable_irq();
}



// Main function.  Initializes the hardware then loops while state changes occur.
int main()
{
	
	InitHardware();
	
	while (true) 
	{
		

		
	}
	
}

	
