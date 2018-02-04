// File:  HAL_PLL.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: PLL and bus clock utilities
// Hardware:  TM4C123 Tiva board

#ifndef PLL_HAL_H
#define PLL_HAL_H

#include <stdint.h>

//------------------------- PLL_Init80MHz ----------------------------
// Initializes the PLL to a bus clock frequency of 80MHz.
// Inputs:  none.
// Outputs:  none.
void PLL_Init80MHz(void);

//----------------------- PLL_GetBusClockFreq ------------------------
// Gets the PLL bus clock frequency.
// Inputs:  none.
// Outputs:  the configured bus clock frequency or zero if uninitialized.
uint32_t PLL_GetBusClockFreq(void);

#endif


