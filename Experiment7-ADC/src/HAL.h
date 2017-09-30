// File:  HAL.h
// Author: JS
// Date:  9/14/17
// Purpose: Hardware abstraction layer constants and prototypes
// Hardware:  TM4C123 Tiva board

#ifndef HAL_H
#define HAL_H

// These are service layer names for the ports.
typedef enum { 
	PORTA = 0, 
	PORTB = 1, 
	PORTC = 2, 
	PORTD = 3, 
	PORTE = 4, 
	PORTF = 5
} PortName_t;	

#include "PLL_HAL.h"
#include "SysTick_HAL.h"
#include "UART_HAL.h"
#include "GPIO_HAL.h"

#endif


