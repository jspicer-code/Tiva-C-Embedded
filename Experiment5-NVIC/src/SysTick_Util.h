// File:  SysTick_Util.h
// Author: JS
// Date:  9/9/17
// Purpose: SysTick and delay utilities
// Hardware:  TM4C123 Tiva board

void SysTick_Init(void);
void SysTick_Wait(uint32_t delay);
void SysTick_Wait10ms(uint32_t delay);
