
#include "../Device.h"

DeviceConfig_t config_;

int main()
{
	config_.sysClkFreq = SYSCLK_80;
	
	// The data pin assignments 0-3 will be mapped to DB4-DB7 of the display
	config_.lcd.rsPin = PINDEF(PORTB, PIN2);
	config_.lcd.rwPin = PINDEF(PORTE, PIN0);
	config_.lcd.enablePin = PINDEF(PORTB, PIN7);
	config_.lcd.dataPins[0] = PINDEF(PORTB, PIN6);
	config_.lcd.dataPins[1] = PINDEF(PORTA, PIN4);
	config_.lcd.dataPins[2] = PINDEF(PORTA, PIN3);
	config_.lcd.dataPins[3] = PINDEF(PORTA, PIN2);
	config_.lcd.waitTimer = TIMER0;
	
	//
	// Per the NRG 40C calibration report:
	//
	//   Slope:   0.761 m/s/Hz => 0.761 m/(s*Hz)
	//   Offset:  0.35 m/s
	//
	config_.anemometer.slope = 0.761f;
	config_.anemometer.offset = 0.35;
	
	config_.edgeTimer = TIMER2;
	config_.edgeTimerPin = PINDEF(PORTB, PIN0);
	config_.displayModeButton = PINDEF(PORTF, PIN4);

	Run(&config_);
}