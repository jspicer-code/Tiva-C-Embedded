// File:  FanController.h
// Author: JSpicer
// Date:  11/24/17
// Purpose: FanController service layer module
// Hardware:  TM4C123 Tiva board

#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <stdint.h>
#include "HAL.h"

// This structure identifies all hardware HAL ports, pins, and modules to use.
typedef struct {
	
	// GPIO port/pin for the heartbeat LED.
	PinDef_t			heartBeatLED;
	
	// GPIO port/pin for the temperature fan mode switch.
	PinDef_t			tempModeSwitch;
	
	// GPIO port/pin manual fan mode switch.
	PinDef_t			manualModeSwitch;
	
	// GPIO port/pin for the fan relay.
	PinDef_t			fanRelay;
	
	// GPIO port/pin for the display mode tachometer switch.
	PinDef_t			tachDisplaySwitch;
	
	// GPIO port/pin for the display mode temperature switch.
	PinDef_t			tempDisplaySwitch;
	
	// SSI module to use for SPI transfer of data to the display shift register.
	SSIModule_t 	displaySSI;
	
	// Timer block for the display refresh timer.
	TimerBlock_t 	displayTimer;
	
	// ADC module/channel for the manual speed mode pot.
	AnalogDef_t		speedPot;
	
	// ADC module/channel for the thermistor.	
	AnalogDef_t		thermistor;
	
	// PWM module to use for the fan speed.
	PWMDef_t			pwm;
	
	// Timer block to user for the RPM counter.
	TimerBlock_t	rpmTimer;
	
	// Timer block for the tachometer input edge counter.
	TimerBlock_t	tachCounter;
	
	// UART for the serial console.
	UART_ID_t			uart;
	
} FanController_IOConfig_t;


//----------------------- FanController_Run --------------------------
// Runs the FanController.  This is the main entry point.  Blocks and 
//   doesn't return.  Should be called by the main() function upon startup.
// Inputs:  pIOConfig - a pointer to the IO configuration that the 
//   FanController should use.
// Outputs:  none
void FanController_Run(FanController_IOConfig_t* pIOConfig);


#endif
