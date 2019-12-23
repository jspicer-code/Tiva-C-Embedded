// File:  device.c
// Author: JS
// Date:  12/1/19
// Purpose: Input capture experiment
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "Device.h"
#include "LCD.h"
#include "CaptureTimer.h"
#include "Switch.h"
#include "utilities/Strings.h"

#define MAX_PERIOD 			(4000) 	// milliseconds, => 250 mHz
#define MIN_FREQUENCY 	(1000.0f / (float)MAX_PERIOD)
#define POLL_INTERVAL 	(100) 	// milliseconds


static LCDDisplay_t display_;
static CaptureTimer_t* capTimer_;
static CaptureTimer_Config_t capTimerConfig_;
static Switch_t dutyCycleButton_;
	
	
// Catch-all error handler.
static void ErrHandler(void) 
{
		// Spin...
		while (1);
}

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
	lcdConfig.rows = 4;
	lcdConfig.columns = 20;

	if (LCD_Initialize(&lcdConfig, &display_) < 0) {
		return false;
	}
	
	// Disable the cursor.
	LCD_EnableCursor(&display_, 0, 0);
	LCD_SetCursorPosition(&display_, 0, 0);
	
	capTimerConfig_.timer = pConfig->edgeTimeTimer;
	capTimerConfig_.timerPin = pConfig->timerPin;
	capTimerConfig_.levelPin = pConfig->levelPin;
	capTimerConfig_.irqPriority = 7;
	capTimerConfig_.measureDutyCycle = true;
	
	capTimer_ = CaptureTimer_Init(&capTimerConfig_);
	if (!capTimer_) {
		return false;
	}
	
	GPIO_EnableDI(pConfig->button.port, pConfig->button.pin, PULL_UP);
	Switch_Init(&dutyCycleButton_, &pConfig->button, SWITCH_CLOSED_LEVEL_LOW);

	__enable_irq();
		
	return true;
}

static void UpdateDisplay(CaptureTimer_PulseStatus_t status, const CaptureTimer_PulseInfo_t* pulse)
{	
	char line1[32] = "";
	char line2[32] = "";
	
	if (status == CAPTIMER_PULSE_NOSIGNAL) {
		strcpy(line1, "No signal");
		strcpy(line2, "");
	}
	else if (status == CAPTIMER_PULSE_INVALID) {
		strcpy(line1, "Out of range");
		strcpy(line2, "");
	}
	else {
		strncat(line1, "F: ", 3);
		dtoa((double)pulse->frequency, &line1[3], 5);
		strncat(line1, " Hz", 3);

		if (capTimerConfig_.measureDutyCycle) {
		
			float dutyCycle = pulse->dutyCycle * 100.f;
			strncat(line2, "D: ", 3);
		
			dtoa((double)dutyCycle, &line2[3], 2);
			strncat(line2, "%", 1);
		}
	}

	pad(line1, ' ', display_.columns + 1);
	LCD_PutString(&display_, line1, 0, 0);
	
	pad(line2, ' ', display_.columns + 1);
	LCD_PutString(&display_, line2, 1, 0);
}

static void EnableDutyCycle(bool enable)
{
	
	CaptureTimer_Terminate(capTimer_);
			
	capTimerConfig_.measureDutyCycle = enable;
	capTimer_ = CaptureTimer_Init(&capTimerConfig_);
	
	if (!capTimer_) {
		ErrHandler();
	}

	capTimerConfig_.measureDutyCycle = enable;
}


static CaptureTimer_PulseStatus_t GetPulse(CaptureTimer_PulseInfo_t* pulse)
{

	static int noSignalCount = 0;
	static const int MAX_NOSIGNAL_COUNT = (MAX_PERIOD / POLL_INTERVAL);
	static CaptureTimer_PulseInfo_t lastPulse = { 0 };
	static bool isLastPulseValid = false;
	
	CaptureTimer_PulseStatus_t status = CaptureTimer_GetPulse(capTimer_ , pulse);

	switch (status) {
		
		case CAPTIMER_PULSE_NOSIGNAL:
		
			if (isLastPulseValid && noSignalCount < MAX_NOSIGNAL_COUNT) {
				*pulse = lastPulse;
				status = CAPTIMER_PULSE_VALID;
				++noSignalCount;
			}
			break;

		case CAPTIMER_PULSE_INVALID:
			isLastPulseValid = false;
			break;
			
		case CAPTIMER_PULSE_VALID:
			
			if (pulse->frequency >= MIN_FREQUENCY) {
				isLastPulseValid = true;
				lastPulse = *pulse;
				noSignalCount = 0;
			} 
			else {
				isLastPulseValid = false;
				status = CAPTIMER_PULSE_NOSIGNAL;
			}
				
			break;
	}
	
	return status;
	
}

int Run(DeviceConfig_t* pConfig)
{

	if (!InitHardware(pConfig)) {
		ErrHandler();
	}
	
	LCD_RawClearDisplay(&display_.raw);

	const int waitDelay = POLL_INTERVAL / 10;
	
	for (;;) {
	
		SysTick_Wait10ms(waitDelay);
		
		// Toggle measurement of the duty cycle when SW2 is released. 
		Switch_Poll(&dutyCycleButton_);
		if (Switch_HasOpened(dutyCycleButton_)) {
			EnableDutyCycle(!capTimerConfig_.measureDutyCycle);
		}
		
		CaptureTimer_PulseInfo_t pulse;
		CaptureTimer_PulseStatus_t status = GetPulse(&pulse);
	
		UpdateDisplay(status, &pulse);
		
	}
	
}
