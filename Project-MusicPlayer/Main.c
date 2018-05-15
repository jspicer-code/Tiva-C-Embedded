// File:  project.c
// Author: jspicer-ltu
// Date:  3/31/18
// Purpose: Main project file.  
// Hardware:  TM4C123 Tiva board

#include "MusicPlayer.h"

MusicPlayer_Config_t config_;
	
int main()
{
	// The display is 20x4.  Using a 4-bit bus, only need to set data pins 4-7.	
	config_.lcd.dataPins[4] = PINDEF(PORTB, PIN7);
	config_.lcd.dataPins[5] = PINDEF(PORTA, PIN4);
	config_.lcd.dataPins[6] = PINDEF(PORTA, PIN3);
	config_.lcd.dataPins[7] = PINDEF(PORTA, PIN2);	
	config_.lcd.rsPin = PINDEF(PORTB, PIN2);
	config_.lcd.rwPin = PINDEF(PORTE, PIN0);
	config_.lcd.enablePin = PINDEF(PORTA, PIN6);
	config_.lcd.dataLen = LCD_DATALEN_4;
	config_.lcd.waitTimer = TIMER0;
	config_.lcd.initByInstruction = 0;
	config_.lcd.rows = 4;
	config_.lcd.columns = 20;
	
	config_.playButton = PINDEF(PORTE, PIN2);
	config_.nextButton = PINDEF(PORTF, PIN0);
	config_.prevButton = PINDEF(PORTF, PIN4);
	
	config_.trackLED[0] = PINDEF(PORTE, PIN3); 
	config_.trackLED[1] = PINDEF(PORTA, PIN5);
	config_.trackLED[2] = PINDEF(PORTB, PIN4);
	config_.trackLED[3] = PINDEF(PORTE, PIN5);
	
	config_.trackPWM[0] = PWMDEF(PWMModule0, PWM0); // PB6	
	config_.trackPWM[1] = PWMDEF(PWMModule0, PWM3);	// PB5
	config_.trackPWM[2] = PWMDEF(PWMModule0, PWM4);	// PE4
	config_.trackPWM[3] = PWMDEF(PWMModule1, PWM3); // PA7
	
	config_.uart = UART1; // Rx=PB0, Tx=PB1
	
	config_.flashBaseAddress = (volatile uint32_t*)0x8000;
	config_.flashBlocks = 224;
	
	config_.noiseTimer = TIMER1;
	
	MusicPlayer_Run(&config_);
	
}
