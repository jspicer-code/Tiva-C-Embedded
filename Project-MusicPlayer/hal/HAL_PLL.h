// File:  HAL_PLL.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: PLL and bus clock utilities
// Hardware:  TM4C123 Tiva board

#ifndef PLL_HAL_H
#define PLL_HAL_H

#include <stdint.h>

// Holds the currently configured bus clock frequency.
extern uint32_t PLL_BusClockFreq;

// Stores the number of ticks in 10ms, 10us, and 100ns based on the bus clock frequency.
extern uint32_t PLL_NumTicksPer10ms;
extern uint32_t PLL_NumTicksPer10us;
extern uint32_t PLL_NumTicksPer100ns;

//------------------------- PLL_Init80MHz ----------------------------
// Initializes the PLL to a bus clock frequency of 80MHz.
// Inputs:  none.
// Outputs:  none.
void PLL_Init80MHz(void);


#endif


