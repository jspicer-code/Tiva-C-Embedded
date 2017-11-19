// File:  PLL_HAL.h
// Author: JS
// Date:  9/14/17
// Purpose: PLL and bus clock utilities
// Hardware:  TM4C123 Tiva board

#ifndef PLL_HAL_H
#define PLL_HAL_H

#include <stdint.h>

extern uint32_t BusClockFreq;

void PLL_Init80MHz(void);

#endif


