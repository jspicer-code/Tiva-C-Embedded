// File:  device.c
// Author: JS
// Date:  7/14/18
// Purpose: RTC device module
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "device.h"
#include "DisplayManager.h"
#include "Switch5Position.h"

// This is the DS1307's slave address.
#define RTC_SLAVE_ADDRESS		0x68

static DeviceConfig_t* pConfig_;

//0=CLOCK mode, 1=SET mode 
static bool setMode_;

void ReadClock(Display_Clock_t* clock)
{
	char data[7];
	
	// Set the current address register to 0x00.  Do not issue a stop condition.
	data[0] = 0x00;
	I2C_MasterWrite(pConfig_->i2cModule, RTC_SLAVE_ADDRESS, data, 1, false, false); 
	
	// Issue a repeated start, and read all date/time registers starting from address 0x00.
	int error = I2C_MasterRead(pConfig_->i2cModule, RTC_SLAVE_ADDRESS, data, 7, true, true); 	
	if (!error) {
		
		clock->seconds = 10 * ((data[0] & 0x70) >> 4);
		clock->seconds += (data[0] & 0x0F);
		
		clock->minutes = 10 * ((data[1] & 0x70) >> 4);
		clock->minutes += (data[1] & 0x0F);

		clock->am_pm = (data[2] & 0x20) >> 5;
		
		clock->hours = 10 * ((data[2] & 0x10) >> 4);
		clock->hours += (data[2] & 0x0F);
		
		clock->day = data[3] & 0x07;
		
		clock->date = 10 * ((data[4] & 0x30) >> 4);
		clock->date += (data[4] & 0x0F);

		clock->month = 10 * ((data[5] & 0x10) >> 4);
		clock->month += (data[5] & 0x0F);

		clock->year = 10 * ((data[6] & 0xF0) >> 4);
		clock->year += (data[6] & 0x0F);

	}
}

void WriteClock(const Display_Clock_t* clock)
{
	char data[8];

	// The first by written is the starting address register, set to 0x0 here.
	// The remaining seven bytes are the register values.
	data[0] = 0x00;
	data[1] = ((clock->seconds / 10) << 4) | (clock->seconds % 10);
	data[2] = ((clock->minutes / 10) << 4) | (clock->minutes % 10);
	data[3] = 0x40 |  (clock->am_pm << 5) | ((clock->hours / 10) << 4) | clock->hours % 10;
	data[4] = clock->day;
	data[5] = ((clock->date / 10) << 4) | (clock->date % 10);
	data[6] = ((clock->month / 10) << 4) | (clock->month % 10);
	data[7] = ((clock->year / 10) << 4) | (clock->year % 10);
	
	I2C_MasterWrite(pConfig_->i2cModule, RTC_SLAVE_ADDRESS, data, 8, false, true); 
	
}


// Called periodically by the timer ISR.
void ClockUpdateCallback(void)
{
	// If not in SET mode (i.e. in CLOCK mode), then update the clock.
	if (!setMode_) {
		Display_Clock_t clock;
		ReadClock(&clock);
		Display_UpdateClock(&clock);
	}
}

static void InitClock(void)
{
	
	// TODO: read the seconds register and if the CH bit is 1, then clear it to start the oscillator.

	// Seems like this only needs to be done once if there is a battery.
//	char data[2];
// Note that with the battery this will clear the seconds register which is not good.
//	data[0] = 0x00; // Seconds Register
//	data[1] = 0x00; // Clear the Clock Halt (CH) bit to enable the oscillator
//	I2C_MasterWrite(pConfig->i2cModule, RTC_SLAVE_ADDRESS, data, 2, false, true); 
	
//	data[0] = 0x07; // Control Register
//	data[1] = 0x10; // Set the SQWE bit to enable the oscillator output for 1Hz
//	I2C_MasterWrite(pConfig->i2cModule, RTC_SLAVE_ADDRESS, data, 2, false, true); 
}


static int InitHardware(DeviceConfig_t* pConfig)
{
	// Save a copy of the configuration.
	pConfig_ = pConfig;
	
	__disable_irq();
	
	PLL_Init(pConfig->sysClkFreq);	
	
	SysTick_Init();
	
	I2C_InitModule(pConfig->i2cModule);
	
	// Use Standard 100Kbps mode.
	I2C_EnableAsMaster(pConfig->i2cModule, 100000, false);
	
	InitClock();
	
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
		return -1;
	}
	
	if (Switch5_Initialize(&pConfig->switches) < 0) {
		return -1;
	}
		
	__enable_irq();
	
	// Call back frequency is 1Hz.
	Timer_Init(TIMER1, TIMER_PERIODIC, 7, ClockUpdateCallback);
	Timer_Start(TIMER1, PLL_BusClockFreq);
	
	return 0;
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
				Display_Clock_t clock;
				Display_GetClock(&clock);
				WriteClock(&clock);
			}
		
			// Toggle modes and turn on/off the input cursor.
			setMode_ = !setMode_;
			Display_EnableCursor(setMode_);
			Display_ResetCursorPosition();
			
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

	if (InitHardware(pConfig)) {
		for (;;);
	}
	
	for (;;) {
		ProcessSwitchState(100);			
		SysTick_Wait10ms(10);
	}
	
}
