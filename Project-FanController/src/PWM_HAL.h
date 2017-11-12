// File:  PWM_HAL.h
// Author: JS
// Date:  10/04/17
// Purpose: PWM utilities
// Hardware:  TM4C123 Tiva board

#ifndef PWM_HAL_H
#define PWM_HAL_H

#include <stdint.h>

typedef enum {
	PWM0 = 0,
	PWM1 = 1,
	PWM2 = 2,
	PWM3 = 3,
	PWM4 = 4,
	PWM5 = 5,
	PWM6 = 6,
	PWM7 = 7,
} PWMChannel;


typedef enum {
	PWMModule0 = 0,
	PWMModule1 = 1
} PWMModule;


void PWM_Enable(PWMModule module, PWMChannel channel, uint16_t period, uint16_t duty);
void PWM_SetDuty(PWMModule module, PWMChannel channel, uint16_t duty);

#endif

