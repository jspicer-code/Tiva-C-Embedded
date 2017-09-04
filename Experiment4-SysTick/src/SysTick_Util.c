#include <stdint.h>
#include "SysTick_Util.h"
#include "tm4c123gh6pm.h"

void SysTick_Init(void)
{
	
	// 1) Disable SysTick during initialization.
	NVIC_ST_CTRL_R = 0;
	
	// 2) Set the RELOAD register to establish a modulo RELOAD + 1 decrement counter
	NVIC_ST_RELOAD_R = 0x00FFFFFF;
	
	// 3) Clear the accumulator
	NVIC_ST_CURRENT_R = 0;
	
	// 4) Set clock source to core clock and enable 
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_ENABLE;
	
}

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

void SysTick_Wait10ms(uint32_t delay)
{
	uint32_t i;
	for (i = 0; i < delay; i++) {
		// NOTE:  This currently assumes a bus frequency of 80MHz!!
		SysTick_Wait(800000);
	}
	
}
