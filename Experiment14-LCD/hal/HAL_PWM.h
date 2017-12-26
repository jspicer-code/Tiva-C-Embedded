// File:  HAL_PWM.h
// Author: JS
// Date:  10/04/17
// Purpose: PWM utilities
// Hardware:  TM4C123 Tiva board

#ifndef PWM_HAL_H
#define PWM_HAL_H

#include <stdint.h>

// Names of the PWM channels
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

// Names of the PWM modules.
typedef enum {
	PWMModule0 = 0,
	PWMModule1 = 1
} PWMModule;

// This is structure is an abtract definition of PWM module/channel combination.
typedef struct {
	PWMModule module;	
	PWMChannel channel;
} PWMDef_t;

// Convenience macro for initializing a PWMDef_t instance.
#define	PWMDEF(module, channel)		((PWMDef_t){ module, channel })

//----------------------- PWM_Enable --------------------------
// Enables the specified PWM module/channel with a given period 
//	and duty cycle.
// Inputs:  module - the PWM module name.
//          channel - the PWM channel name.
//          period - the pulse period.
//          duty - the number of bus cycles for initial duty cycle 
// Outputs:  none.
void PWM_Enable(PWMModule module, PWMChannel channel, uint16_t period, uint16_t duty);


//----------------------- PWM_SetDuty --------------------------
// Set the duty cycle of an enabled PWM module/channel
// Inputs:  module - the PWM module name.
//          channel - the PWM channel name.
//          duty - the number of bus cycles for the duty cycle.
// Outputs:  none.
void PWM_SetDuty(PWMModule module, PWMChannel channel, uint16_t duty);

#endif

