// File:  HAL.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: Hardware abstraction layer constants and prototypes
// Hardware:  TM4C123 Tiva board

#ifndef HAL_H
#define HAL_H

#include <stdint.h>

// These are service layer names for the HW ports.
typedef enum { 
	PORTA = 0, 
	PORTB = 1, 
	PORTC = 2, 
	PORTD = 3, 
	PORTE = 4, 
	PORTF = 5
} PortName_t;	

// These are service layer names for the HW pins.
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

typedef struct {
	PortName_t		port;
	PinName_t			pin;
}	PinDef_t;

#define	PINDEF(port, pin)		((PinDef_t){ port, pin })

#define PIN_0		0x01
#define PIN_1		0x02
#define PIN_2		0x04
#define PIN_3		0x08
#define PIN_4		0x10
#define PIN_5		0x20
#define PIN_6		0x40
#define PIN_7		0x80

#include "HAL_PLL.h"
#include "HAL_SysTick.h"
#include "HAL_UART.h"
#include "HAL_GPIO.h"
#include "HAL_ADC.h"
#include "HAL_PWM.h"
#include "HAL_Timer.h"
#include "HAL_SPI.h"

#endif


