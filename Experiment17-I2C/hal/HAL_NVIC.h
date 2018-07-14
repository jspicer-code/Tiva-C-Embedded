// File:  HAL_NVIC.h
// Author: JSpicer
// Date:  7/14/18
// Purpose: NVIC utilities
// Hardware:  TM4C123, TM4C1294 Tiva board

#ifndef HAL_NVIC_H
#define HAL_NVIC_H

#include <stdint.h>

void NVIC_EnableIRQ(uint8_t irq, uint8_t priority);
void NVIC_DisableIRQ(uint8_t irq);

#endif
