// File:  device.c
// Author: JS
// Date:  7/14/18
// Purpose: Frequency measurment experiment
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "Device.h"
#include "LCD.h"
#include "FrequencyTimer.h"
#include "utilities/Strings.h"

static LCDDisplay_t display_;
static FrequencyTimer_t freqTimer_;

static bool InitHardware(DeviceConfig_t* pConfig)
{
	__disable_irq();
	
	PLL_Init(pConfig->sysClkFreq);	
	
	if (SysTick_Init() != 0) {
		return false;
	}
	
	LCDPinConfig_t lcd;	
	lcd.rsPin = pConfig->lcd.rsPin;
	lcd.rwPin = pConfig->lcd.rwPin;
	lcd.enablePin = pConfig->lcd.enablePin;
	lcd.dataPins[4] = pConfig->lcd.dataPins[0];
	lcd.dataPins[5] = pConfig->lcd.dataPins[1];
	lcd.dataPins[6] = pConfig->lcd.dataPins[2];
	lcd.dataPins[7] = pConfig->lcd.dataPins[3];
	lcd.waitTimer = pConfig->lcd.waitTimer;
	lcd.dataLen = LCD_DATALEN_4;
	lcd.initByInstruction = 0;
	lcd.rows = 1;
	lcd.columns = 16;
	
	if (Timer_Init(lcd.waitTimer, TIMER_ONESHOT, (void*)0, (void*)0)) {
		return false;
	}
	
	if (LCD_Initialize(&display_, &lcd, lcd.rows, lcd.columns) < 0) {
		return false;
	}
	
	// Disable the cursor.
	LCD_EnableCursor(&display_, 0, 0);
	LCD_SetCursorPosition(&display_, 0, 0);
	
	if (FrequencyTimer_Enable(pConfig->edgeTimer, 7, &pConfig->edgeTimerPin, 0.25f, &freqTimer_)) {
		return false;
	}

	__enable_irq();
		
	return true;
}

static void UpdateDisplay(double frequency)
{	
	char line1[32];
	
	dtoa(frequency, line1, 5);
	strncat(line1, " Hz", 3);

	pad(line1, ' ', 17);
	LCD_PutString(&display_, line1, 0, 0);
}


int Run(DeviceConfig_t* pConfig)
{
	
	if (!InitHardware(pConfig)) {
		for (;;);
	}
	
	LCD_RawClearDisplay(&display_.raw);

	for (;;) {
	
		SysTick_Wait10ms(5);
	
		double frequency = FrequencyTimer_GetFrequency(&freqTimer_);
		UpdateDisplay(frequency);
		
	}
	
}
