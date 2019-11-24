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
static FrequencyTimer_t* freqTimer_;

static bool InitHardware(DeviceConfig_t* pConfig)
{
	__disable_irq();
	
	PLL_Init(pConfig->sysClkFreq);	
	
	if (SysTick_Init() != 0) {
		return false;
	}
	
	LCDConfig_t lcdConfig;	
	lcdConfig.rsPin = pConfig->lcd.rsPin;
	lcdConfig.rwPin = pConfig->lcd.rwPin;
	lcdConfig.enablePin = pConfig->lcd.enablePin;
	lcdConfig.dataPins[4] = pConfig->lcd.dataPins[0];
	lcdConfig.dataPins[5] = pConfig->lcd.dataPins[1];
	lcdConfig.dataPins[6] = pConfig->lcd.dataPins[2];
	lcdConfig.dataPins[7] = pConfig->lcd.dataPins[3];
	lcdConfig.waitTimer = pConfig->lcd.waitTimer;
	lcdConfig.dataLen = LCD_DATALEN_4;
	lcdConfig.initByInstruction = 0;
	lcdConfig.rows = 1;
	lcdConfig.columns = 16;

	if (LCD_Initialize(&lcdConfig, &display_) < 0) {
		return false;
	}
	
	// Disable the cursor.
	LCD_EnableCursor(&display_, 0, 0);
	LCD_SetCursorPosition(&display_, 0, 0);
	
	FrequencyTimerConfig_t freqTimerConfig;
	freqTimerConfig.timer = pConfig->edgeTimeTimer;
	freqTimerConfig.pin = pConfig->edgeTimePin;
	freqTimerConfig.priority = 7;

	freqTimer_ = FrequencyTimer_Enable(&freqTimerConfig);
	if (!freqTimer_) {
		return false;
	}

	__enable_irq();
		
	return true;
}

static void UpdateDisplay(float frequency)
{	
	char line1[32];
	
	if (frequency == 0.0f) {
		strcpy(line1, "No signal");
	}
	else if (frequency < 0.0f) {
		strcpy(line1, "Out of range");
	}
	else {
		dtoa((double)frequency, line1, 5);
		strncat(line1, " Hz", 3);
	}

	pad(line1, ' ', 17);
	LCD_PutString(&display_, line1, 0, 0);
}


float GetFrequency(int pollInterval)
{
	const int maxPeriod = 4000; // => 250 mHz
	const float minFrequency = 1000.0f / (float)maxPeriod;
	
	static int noSignalTimer = 0;
	static float lastFrequency = 0.0f;
	
	float frequency = FrequencyTimer_GetFrequency(freqTimer_);
	if (frequency > 0.0f) {
		
		if (frequency >= minFrequency) {
			lastFrequency = frequency;
			noSignalTimer = 0;
		}
		else {
			frequency = 0.0f;			
		}

	}
	else if (frequency == 0.0f) {
		
		if (noSignalTimer < maxPeriod) {
			frequency = lastFrequency;
			noSignalTimer += pollInterval;
		}
		else {
			lastFrequency = 0.0f; 				
		}
	}
	
	return frequency;
	
}

int Run(DeviceConfig_t* pConfig)
{
	
	if (!InitHardware(pConfig)) {
		for (;;);
	}
	
	LCD_RawClearDisplay(&display_.raw);

	const int waitDelay = 10;
	
	for (;;) {
	
		SysTick_Wait10ms(waitDelay);
		
		float frequency = GetFrequency(10 * waitDelay);
	
		UpdateDisplay(frequency);
		
	}
	
}
