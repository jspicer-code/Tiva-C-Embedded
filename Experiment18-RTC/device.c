// File:  device.c
// Author: JS
// Date:  7/14/18
// Purpose: RTC experiment
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "Device.h"
#include "RTC.h"
#include "DisplayManager.h"
#include "Switch5Position.h"

// 0=CLOCK mode, 1=SET mode 
static bool setMode_;

void UpdateClock(void)
{
	RTC_Clock_t clock;
	RTC_Read(&clock);
	Display_UpdateClock(&clock);
}

// Called periodically by the timer ISR.
void ClockTimerCallback(void)
{
	// If not in SET mode (i.e. in CLOCK mode), then update the clock.
	if (!setMode_) {
		UpdateClock();
	}
}

static bool InitHardware(DeviceConfig_t* pConfig)
{
	__disable_irq();
	
	PLL_Init(pConfig->sysClkFreq);	
	
	SysTick_Init();
	
	I2C_InitModule(pConfig->i2cModule);
	
	// Use Standard 100Kbps mode.
	I2C_EnableAsMaster(pConfig->i2cModule, 100000, false);
	
	bool isReset;
	if (!RTC_Init(pConfig->i2cModule, true, &isReset)) {
		return false;
	}

	setMode_ = isReset;
	
	LCDPinConfig_t lcd;	
	lcd.rsPin = pConfig->lcd.rsPin;
	lcd.rwPin = pConfig->lcd.rwPin;
	lcd.enablePin = pConfig->lcd.enablePin;
	lcd.dataPins[4] = pConfig->lcd.dataPins[0];
	lcd.dataPins[5] = pConfig->lcd.dataPins[1];
	lcd.dataPins[6] = pConfig->lcd.dataPins[2];
	lcd.dataPins[7] = pConfig->lcd.dataPins[3];
	lcd.dataLen = LCD_DATALEN_4;
	lcd.waitTimer = TIMER0;
	lcd.initByInstruction = 0;
	lcd.rows = 2;
	lcd.columns = 16;
	if (Display_Init(&lcd)) {
		return false;
	}
	
	if (Switch5_Initialize(&pConfig->switches) < 0) {
		return false;
	}
		
	__enable_irq();
	
	// Call back frequency is 1Hz.
	Timer_Init(TIMER1, TIMER_PERIODIC, 7, ClockTimerCallback);
	Timer_Start(TIMER1, PLL_BusClockFreq);
	
	return true;
}


void ProcessSwitchState(int pollPeriod)
{
	static int centerDownTime = 0;
		
	// Get the current switch state.
	uint32_t sw = Switch5_Poll();
	
	// If the center switch is closed...
	if (SWITCH5_ISCLOSED(sw, SWITCH5_CENTER) && !SWITCH5_HASCLOSED(sw, SWITCH5_CENTER)) {
		
		centerDownTime += pollPeriod;
		
		// If then center switch has been closed for a couple seconds, then change modes.
		if (centerDownTime > 2000) {
				
			// If currently in SET mode, then get the edited values and write them the RTC.
			if (setMode_) {
				RTC_Clock_t clock;
				Display_GetClock(&clock);
				RTC_Write(&clock);
			}
		
			// Toggle modes and turn on/off the input cursor.
			setMode_ = !setMode_;
			Display_EnableCursor(setMode_);
			
			centerDownTime = 0;
		}
	}
	else if (setMode_) {
	
		// The Parallax switch is rotated 90 degrees counter-clockwise to fit on the breadboard,
		//	so the right contact is UP, the up contact is LEFT, etc. 
		if (SWITCH5_HASCLOSED(sw, SWITCH5_DOWN)) {
			Display_NextField();
		}
		else if (SWITCH5_HASCLOSED(sw, SWITCH5_UP)) {
			Display_PrevField();
		}
		else if (SWITCH5_HASCLOSED(sw, SWITCH5_RIGHT)) {
			Display_NextValue();
		}
		else if (SWITCH5_HASCLOSED(sw, SWITCH5_LEFT)) {
			Display_PrevValue();
		}
		
		centerDownTime = 0;	
	}
	
}

int Run(DeviceConfig_t* pConfig)
{

	if (!InitHardware(pConfig)) {
		for (;;);
	}
	
	if (setMode_) {
		Display_EnableCursor(true);
		UpdateClock();
	}
	
	for (;;) {
		ProcessSwitchState(100);			
		SysTick_Wait10ms(10);
	}
	
}
