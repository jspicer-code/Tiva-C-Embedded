// File:  HAL.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: Hardware abstraction layer constants and prototypes
// Hardware:  TM4C123 Tiva board

#ifndef HAL_H
#define HAL_H

#include <stdint.h>

// Names for the IO ports.
typedef enum { 
	PORTA = 0, 
	PORTB = 1, 
	PORTC = 2, 
	PORTD = 3, 
	PORTE = 4, 
	PORTF = 5
} PortName_t;	

// Names for the IO pins.  These are also bit flags and can
//	OR'd together for pin maps.
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

// Include the other HAL modules.
#include "HAL_PLL.h"
#include "HAL_SysTick.h"
#include "HAL_UART.h"
#include "HAL_GPIO.h"
#include "HAL_ADC.h"
#include "HAL_PWM.h"
#include "HAL_Timer.h"
#include "HAL_SPI.h"
#include "HAL_Flash.h"

#endif


