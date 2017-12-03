// File:  HAL_SysTick.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: SysTick and delay utilities
// Hardware:  TM4C123 Tiva board

#ifndef SYSTICK_HAL_H
#define SYSTICK_HAL_H

#include <stdint.h>

int SysTick_Init(void);
void SysTick_Wait(uint32_t delay);
void SysTick_Wait10ms(uint32_t delay);
void SysTick_Wait10us(uint32_t delay);


#endif


