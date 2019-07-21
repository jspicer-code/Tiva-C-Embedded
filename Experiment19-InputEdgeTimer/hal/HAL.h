// File:  HAL.h
// Author: JSpicer
// Date:  6/23/18
// Purpose: Hardware abstraction layer constants and prototypes
// Hardware:  TM4C123, TM4C1294 Tiva board

#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>

// Names for the IO ports.
typedef enum { 
	
	// TM4C123 and TM4C1294
	PORTA = 0, 
	PORTB = 1, 
	PORTC = 2, 
	PORTD = 3, 
	PORTE = 4, 
	PORTF = 5,

	// TM4C1294 only
	PORTG = 6,	
	PORTH = 7,
	PORTJ = 8,
	PORTK = 9,
	PORTL = 10,
	PORTM = 11,
	PORTN = 12,
	PORTP = 13,
	PORTQ = 14
} PortName_t;	

// Names for the IO pins.  These are also bit flags and can
//	OR'd together as pin maps.
typedef enum { 
	PIN0 = 0x01,
	PIN1 = 0x02,
	PIN2 = 0x04,
	PIN3 = 0x08,
	PIN4 = 0x10,
	PIN5 = 0x20,
	PIN6 = 0x40,
	PIN7 = 0x80
} PinName_t;	

// This is an abstract defintion for a port/pin combination.
typedef struct {
	PortName_t		port;
	PinName_t			pin;
}	PinDef_t;

// Convenience macro to initialize a PinDef_t instance.
#define	PINDEF(port, pin)		((PinDef_t){ port, pin })

#include "HAL_Config.h"

// Include the other HAL modules.
#include "HAL_PLL.h"
#include "HAL_SysTick.h"
#include "HAL_GPIO.h"
#include "HAL_NVIC.h"
#include "HAL_I2C.h"
#include "HAL_Timer.h"
#include "HAL_SPI.h"

#endif


