// File:  FanController.h
// Author: JSpicer
// Date:  11/24/17
// Purpose: FanController service layer module
// Hardware:  TM4C123 Tiva board

#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <stdint.h>
#include "HAL.h"

typedef struct {
	
	PinDef_t			heartBeatLED;
	PinDef_t			autoModeSwitch;
	PinDef_t			manualModeSwitch;
	PinDef_t			fanRelay;
	PinDef_t			tachDisplaySwitch;
	PinDef_t			tempDisplaySwitch;
	
	SSIModule_t 	displaySSI;
	TimerBlock_t 	displayTimer;
	
	AnalogDef_t		speedPot;
	AnalogDef_t		thermistor;
	
	PWMDef_t			pwm;
	
	TimerBlock_t	rpmTimer;
	TimerBlock_t	tachCounter;
	
	UART_ID_t			uart;
	
} FanController_IOConfig_t;


void FanController_Run(FanController_IOConfig_t* pIOConfig);


#endif
