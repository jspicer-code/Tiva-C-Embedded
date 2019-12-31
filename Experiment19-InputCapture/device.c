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

#define POLL_INTERVAL 						(250) 	// milliseconds
#define MAX_PULSE_PERIOD 					(4000) 	// milliseconds, => 250 mHz
#define MIN_PULSE_FREQUENCY 			(1000.0f / (float)MAX_PULSE_PERIOD)
#define MAX_DETECTION_FAILURES 		(MAX_PULSE_PERIOD / POLL_INTERVAL)


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

static void UpdateDisplay(const CaptureTimer_PulseInfo_t* pulse, int failures)
{	
	static const float FAILURES_PER_ELLIPSIS_DOT = (16.0f / (float)MAX_DETECTION_FAILURES);
	
	char line1[32] = "";
	char line2[32] = "";
	char line4[32] = "";
	
	if (pulse->frequency <= 0.0f) {
		strcpy(line1, "No pulse detected");
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
		
		if (failures > 0) {
			strcpy(line4, "Wait");
			float ellipsisLength = (float)failures * FAILURES_PER_ELLIPSIS_DOT;
			for (int i = 0; i < (int)ellipsisLength; i++) {
				strncat(line4, ".", 1);
			}
		}
	
	}

	pad(line1, ' ', display_.columns + 1);
	LCD_PutString(&display_, line1, 0, 0);
	
	pad(line2, ' ', display_.columns + 1);
	LCD_PutString(&display_, line2, 1, 0);

	pad(line4, ' ', display_.columns + 1);
	LCD_PutString(&display_, line4, 3, 0);
	
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


static int GetPulse(CaptureTimer_PulseInfo_t* pulse)
{
	static int failures = 0;
	static CaptureTimer_PulseInfo_t lastPulse = { 0 };
	
	CaptureTimer_GetPulse(capTimer_, pulse);
	
	if (pulse->frequency >= MIN_PULSE_FREQUENCY) {
		lastPulse = *pulse;
		failures = 0;
	} 
	else if (failures++ < MAX_DETECTION_FAILURES) {
		*pulse = lastPulse;
	}
	else {
		pulse->frequency = pulse->dutyCycle = 0.0f;
	}
	
	return failures;
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
		int failures = GetPulse(&pulse);
	
		UpdateDisplay(&pulse, failures);
	}
	
}
