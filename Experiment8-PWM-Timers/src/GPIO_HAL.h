// File:  GPIO_HAL.h
// Author: JS
// Date:  9/14/17
// Purpose: GPIO hardware abstraction layer
// Hardware:  TM4C123 Tiva board

#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>

#define PA2 (*((volatile uint32_t*)0x42087F88)) 
#define PA3 (*((volatile uint32_t*)0x42087F8C)) 
#define PA4 (*((volatile uint32_t*)0x42087F90)) 
#define PA5 (*((volatile uint32_t*)0x42087F94)) 
#define PA6 (*((volatile uint32_t*)0x42087F98)) 
#define PA7 (*((volatile uint32_t*)0x42087F9C)) 

#define PB2 (*((volatile uint32_t*)0x420A7F88)) 
#define PB3 (*((volatile uint32_t*)0x420A7F8C)) 
#define PB4 (*((volatile uint32_t*)0x420A7F90)) 
#define PB5 (*((volatile uint32_t*)0x420A7F94)) 
#define PB6 (*((volatile uint32_t*)0x420A7F98)) 
#define PB7 (*((volatile uint32_t*)0x420A7F9C)) 

#define PE0 (*((volatile uint32_t*)0x42487F80))
#define PE1 (*((volatile uint32_t*)0x42487F84))
#define PE2 (*((volatile uint32_t*)0x42487F88))
#define PE3 (*((volatile uint32_t*)0x42487F8C))
#define PE4 (*((volatile uint32_t*)0x42487F90))
#define PE5 (*((volatile uint32_t*)0x42487F94))

#define PF0 (*((volatile uint32_t*)0x424A7F80))
#define PF1 (*((volatile uint32_t*)0x424A7F84))
#define PF2 (*((volatile uint32_t*)0x424A7F88))
#define PF3 (*((volatile uint32_t*)0x424A7F8C))
#define PF4 (*((volatile uint32_t*)0x424A7F90))

#define PIN_0		0x01
#define PIN_1		0x02
#define PIN_2		0x04
#define PIN_3		0x08
#define PIN_4		0x10
#define PIN_5		0x20
#define PIN_6		0x40
#define PIN_7		0x80

typedef enum DO_Drive { DRIVE_2MA, DRIVE_4MA, DRIVE_8MA } DO_Drive_t;
typedef enum DI_Pull { PULL_NONE, PULL_UP, PULL_DOWN } DI_Pull_t;

void GPIO_InitPort(PortName_t port);
void GPIO_EnableDO(PortName_t port, uint8_t pinMap, DO_Drive_t drive);
void GPIO_EnableDI(PortName_t port, uint8_t pinMap, DI_Pull_t pull);
void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl);
void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap);

#endif
