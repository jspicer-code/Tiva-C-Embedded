// File:  HAL_SysTick.c
// Author: JSpicer
// Date:  9/9/17
// Purpose: SysTick and delay utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "HAL.h"



//----------------------- SysTick_Init --------------------------
// Initializes the SysTick bus clock timer.
// Inputs:  none.
// Outputs:  none.
int SysTick_Init(void)
{
	// Return failure if the PLL hasn't been initialized.
	if (PLL_BusClockFreq == 0) {
		return -1;
	}
	
	// 1) Disable SysTick during initialization.
	NVIC_ST_CTRL_R = 0;
	
	// 2) Set the RELOAD register to establish a modulo RELOAD + 1 decrement counter
	NVIC_ST_RELOAD_R = 0x00FFFFFF;
	
	// 3) Clear the accumulator
	NVIC_ST_CURRENT_R = 0;
	
	// 4) Set clock source to core clock and enable 
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_ENABLE;
	
	// Success
	return 0;
}

//----------------------- SysTick_Wait --------------------------
// Waits (blocks) for a given number of bus cycles.
// Inputs:  delay - the number of bus cycles to wait.
// Outputs:  none.
void SysTick_Wait(uint32_t delay)
{
	// Delay in number of bus cycle period units
	NVIC_ST_RELOAD_R = delay - 1;
	
	// Clear CURRENT
	NVIC_ST_CURRENT_R = 0; 
	
	// Wait for COUNT flag to be set upon rollover.
	while ((NVIC_ST_CTRL_R & NVIC_ST_CTRL_COUNT) == 0) {
	}
	
}

//---------------------- SysTick_Wait10ms--------------------------
// Waits (blocks) for a given number of 10 millisecond intervals.
// Inputs:  delay - the number of 10ms intervals to wait.
// Outputs:  none.
void SysTick_Wait10ms(uint32_t delay)
{
	uint32_t i;
	for (i = 0; i < delay; i++) {
		SysTick_Wait(PLL_NumTicksPer10ms);
	}
}

//---------------------- SysTick_Wait10us--------------------------
// Waits (blocks) for a given number of 10 microsecond intervals.
// Inputs:  delay - the number of 10us intervals to wait.
// Outputs:  none.
void SysTick_Wait10us(uint32_t delay)
{
	uint32_t i;
	for (i = 0; i < delay; i++) {
		SysTick_Wait(PLL_NumTicksPer10us);
	}
}
