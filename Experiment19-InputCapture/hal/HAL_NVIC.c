// File:  HAL_NVIC.c
// Author: JSpicer
// Date:  7/14/18
// Purpose: NVIC utilities
// Hardware:  TM4C123, TM4C1294 Tiva board

#include "HAL_NVIC.h"

#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else
#include "tm4c123gh6pm.h"
#endif

static void SetPriority(uint8_t irq, uint8_t priority)
{
	// Divide the IRQ by four.  This selects the address of PRI0-PRI34.
	uint8_t offset = (irq >> 2);
	volatile uint32_t *address = 	(&NVIC_PRI0_R + offset);  

	// Shift left once, preserving bits 1-3 to left align for the assigment below.
	priority = (priority << 1) & 0x0E;

	// There are four priority fields per address and each occupies
	//	the upper nibble of its respective byte.
	uint32_t bit = ((irq % 4) * 8) + 4;
	uint32_t map = (uint32_t)priority << bit;
	*address |= map;
}

void NVIC_EnableIRQ(uint8_t irq, uint8_t priority)
{
	
	SetPriority(irq, priority);
	
	// This selects the address of EN0-EN4
	uint8_t offset = (irq / 32);
	volatile uint32_t *address = 	(&NVIC_EN0_R + offset);  
	
	// Set the irq bit in the corresponding to enable register.
	uint32_t bit = irq % 32;
	*address |= (0x1 << bit);
	
}

void NVIC_DisableIRQ(uint8_t irq)
{
	// This selects the address of DIS0-DIS4
	uint8_t offset = (irq / 32);
	volatile uint32_t *address = 	(&NVIC_DIS0_R + offset);  
	
	// Set the irq bit in the corresponding to disable register.
	uint32_t bit = irq % 32;
	*address |= (0x1 << bit);
	
}




