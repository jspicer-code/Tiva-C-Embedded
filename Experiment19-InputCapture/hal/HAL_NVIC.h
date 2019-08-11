// File:  HAL_NVIC.h
// Author: JSpicer
// Date:  7/14/18
// Purpose: NVIC utilities
// Hardware:  TM4C123, TM4C1294 Tiva board

#ifndef HAL_NVIC_H
#define HAL_NVIC_H

#include <stdint.h>

extern void* NVIC_ISRData[];

void NVIC_EnableIRQ(uint8_t irq, uint8_t priority, void* isrData);
void NVIC_DisableIRQ(uint8_t irq);

__forceinline void* NVIC_GetActiveISRData()
{
	register uint32_t _ipsr __asm("ipsr");
	return NVIC_ISRData[_ipsr - 16];
}


#endif
