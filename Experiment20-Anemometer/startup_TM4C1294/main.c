
#include "../Device.h"

DeviceConfig_t config_;

int main()
{
	config_.sysClkFreq = SYSCLK_80;
		
	// The data pin assignments 0-3 will be mapped to DB4-DB7 of the display
	config_.lcd.dataPins[0] = PINDEF(PORTD, PIN3);
	config_.lcd.dataPins[1] = PINDEF(PORTC, PIN7);
	config_.lcd.dataPins[2] = PINDEF(PORTB, PIN2);
	config_.lcd.dataPins[3] = PINDEF(PORTB, PIN3);
	config_.lcd.rsPin = PINDEF(PORTE, PIN4);
	config_.lcd.rwPin = PINDEF(PORTC, PIN4);
	config_.lcd.enablePin = PINDEF(PORTC, PIN5);
	config_.lcd.waitTimer = TIMER0;
	
	//
	// Per the NRG 40C calibration report:
	//
	//   Slope:   0.761 m/s/Hz => 0.761 m/(s*Hz)
	//   Offset:  0.35 m/s
	//	
	config_.anemometer.slope = 0.761f;
	config_.anemometer.offset = 0.35;
	
	config_.edgeTimer = TIMER1;
	config_.edgeTimerPin = PINDEF(PORTD, PIN2);
	config_.displayModeButton = PINDEF(PORTJ, PIN0);
	
	Run(&config_);
}

