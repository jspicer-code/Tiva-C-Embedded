// File:  project.c
// Author: JS
// Date:  11/18/17
// Purpose:  main function.  It all starts here.
// Hardware:  TM4C123 Tiva board

#include "FanController.h"

int main()
{
		
	// Assign the ports, pins and modules that the FanController will use.
	//	Passing in the IO onfiguration will make it possible to use a different
	//	IO configuration rather than hardcoding it inside the module itself.
	FanController_IOConfig_t ioConfig;
	ioConfig.heartBeatLED = PINDEF(PORTF, PIN3);
	ioConfig.tempModeSwitch = PINDEF(PORTA, PIN4);
	ioConfig.manualModeSwitch = PINDEF(PORTB, PIN6);
	ioConfig.fanRelay = PINDEF(PORTA, PIN7);
	ioConfig.tachDisplaySwitch = PINDEF(PORTB, PIN7);
	ioConfig.tempDisplaySwitch = PINDEF(PORTB, PIN2);	
	ioConfig.displaySSI = SSI0;
	ioConfig.displayTimer = TIMER0;
	ioConfig.speedPot = ANALOGDEF(ADC0, AIN3);
	ioConfig.thermistor = ANALOGDEF(ADC1, AIN10);
	ioConfig.pwm = PWMDEF(PWMModule0, PWM3);
	ioConfig.rpmTimer = TIMER1;
	ioConfig.tachCounter = TIMER2;
	ioConfig.uart = UART5;
	
	// Run the FanContoller...
	FanController_Run(&ioConfig);
	
}
