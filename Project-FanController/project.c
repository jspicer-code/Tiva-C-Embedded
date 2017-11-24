// File:  project.c
// Author: JS
// Date:  11/18/17
// Purpose: FanController project
// Hardware:  TM4C123 Tiva board


#include "FanController.h"


int main()
{
	
	FanController_Config_t	config;
	
	config.heartBeatLED = PINDEF(PORTF, PIN3);
	config.autoModeSwitch = PINDEF(PORTA, PIN4);
	config.manualModeSwitch = PINDEF(PORTB, PIN6);
	config.fanRelay = PINDEF(PORTA, PIN7);
	config.tachDisplaySwitch = PINDEF(PORTB, PIN7);
	config.tempDisplaySwitch = PINDEF(PORTB, PIN2);
	
	config.displaySSI = SSI0;
	config.displayTimer = TIMER0;
	
	config.speedPot = ANALOGDEF(ADC0, AIN3);
	config.thermistor = ANALOGDEF(ADC1, AIN10);
	
	config.pwm = PWMDEF(PWMModule0, PWM3);
	
	config.rpmTimer = TIMER1;
	config.tachCounter = TIMER2;
	
	config.uart = UART5;
	
	FanController_Run(&config);
	
}
