// File:  HAL_PLL.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: PLL and bus clock utilities
// Hardware:  TM4C123 and TM4C1294 Tiva board

#ifndef PLL_HAL_H
#define PLL_HAL_H

#include <stdint.h>
#include "HAL_Config.h"

typedef enum { 
	
	SYSCLK_80,
	
#if (halCONFIG_1294 == 1)
	SYSCLK_120 
#endif
	
} SysClkFreq_t;

// Holds the currently configured bus clock frequency.
extern uint32_t PLL_BusClockFreq;

// Stores the number of ticks in 10ms, 10us, and 100ns based on the bus clock frequency.
extern uint32_t PLL_NumTicksPer10ms;
extern uint32_t PLL_NumTicksPer10us;
extern uint32_t PLL_NumTicksPer100ns;

//------------------------- PLL_Init ----------------------------
// Initializes the PLL to generate the requested SysClk frequency.
// Inputs:  freq - the requested frequency.
// Outputs:  none.
void PLL_Init(SysClkFreq_t freq);


#endif


