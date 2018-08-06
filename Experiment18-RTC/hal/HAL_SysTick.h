// File:  HAL_SysTick.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: SysTick and delay utilities
// Hardware:  TM4C123, TM4C1294 Tiva board

#ifndef SYSTICK_HAL_H
#define SYSTICK_HAL_H

#include <stdint.h>

//----------------------- SysTick_Init --------------------------
// Initializes the SysTick bus clock timer.
// Inputs:  none.
// Outputs:  none.
int SysTick_Init(void);

//----------------------- SysTick_Wait --------------------------
// Waits (blocks) for a given number of bus cycles.
// Inputs:  delay - the number of bus cycles to wait.
// Outputs:  none.
void SysTick_Wait(uint32_t delay);

//---------------------- SysTick_Wait10ms--------------------------
// Waits (blocks) for a given number of 10 millisecond intervals.
// Inputs:  delay - the number of 10ms intervals to wait.
// Outputs:  none.
void SysTick_Wait10ms(uint32_t delay);

//---------------------- SysTick_Wait10us--------------------------
// Waits (blocks) for a given number of 10 microsecond intervals.
// Inputs:  delay - the number of 10us intervals to wait.
// Outputs:  none.
void SysTick_Wait10us(uint32_t delay);


#endif


