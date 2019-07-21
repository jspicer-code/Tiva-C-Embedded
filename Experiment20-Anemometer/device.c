// File:  device.c
// Author: JS
// Date:  7/14/18
// Purpose: Anemometer experiment
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "Device.h"
#include "LCD.h"
#include "Switch.h"
#include "FrequencyTimer.h"
#include "utilities/Strings.h"

#define CONVERSION_MPH 		(2.23694f)
#define CONVERSION_KNOTS 	(1.94385f)

#define	DISPLAY_MODE_MPS  	0
#define	DISPLAY_MODE_MPH  	1
#define DISPLAY_MODE_KNOTS 	2
#define DISPLAY_MODES				3

static const char* windDescriptions_[] = {
	"Calm",
	"Light air",
	"Light breeze",
	"Gentle breeze",
	"Mod. breeze",
	"Fresh breeze",
	"Strong breeze",
	"Near gale",
	"Gale",
	"Strong gale",
	"Storm",
	"Violent Stm.",
	"Hurricane"
};


static LCDDisplay_t display_;
static FrequencyTimer_t freqTimer_;
static Switch_t button_;
static int DisplayMode_ = DISPLAY_MODE_MPS;


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
	lcd.rows = 2;
	lcd.columns = 16;
	
	if (Timer_Init(lcd.waitTimer, TIMER_ONESHOT, (void*)0, (void*)0)) {
		return false;
	}
	
	if (LCD_Initialize(&display_, &lcd, lcd.rows, lcd.columns) < 0) {
		return false;
	}
	
	GPIO_EnableDI(pConfig->displayModeButton.port, pConfig->displayModeButton.pin, PULL_UP);
	Switch_Init(&button_, &pConfig->displayModeButton, SWITCH_CLOSED_LEVEL_LOW);
		
	// Disable the cursor.
	LCD_EnableCursor(&display_, 0, 0);
	LCD_SetCursorPosition(&display_, 0, 0);
	
	// The maximum interval before "no signal" is detected will be 4 seconds => 250 mHz ~ 0.54 m/s
	if (FrequencyTimer_Enable(pConfig->edgeTimer, &pConfig->edgeTimerPin, 4 * PLL_BusClockFreq, 7, &freqTimer_)) {
		return false;
	}

	__enable_irq();
		
	return true;
}

static int GetWindForce(float metersPerSecond, float minimum)
{
	// If the speed is less than or equal to the minimum that can be detected,
	// then the force is zero regardless of the Beaufort scale minimum.
	if (metersPerSecond < 0.3f || metersPerSecond <= minimum) {
		return 0;	// Calm
	}
	
	if (metersPerSecond >= 0.3f && metersPerSecond < 1.6f) {
		return 1; // Light air
	}
	
	if (metersPerSecond >= 1.6f && metersPerSecond < 3.4f) {
		return 2; // Light breeze
	}
	
	if (metersPerSecond >= 3.4f && metersPerSecond < 5.5f) {
		return 3; // Gentle breeze
	}
	
	if (metersPerSecond >= 5.5f && metersPerSecond < 8.0f) {
		return 4;	// Moderate breeze
	}
	
	if (metersPerSecond >= 8.0f && metersPerSecond < 10.8f) {
		return 5;	// Fresh breeze
	}
	
	if (metersPerSecond >= 10.8f && metersPerSecond < 13.9f) {
		return 6;	// Strong breeze
	}
	
	if (metersPerSecond >= 13.9f && metersPerSecond < 17.2f) {
		return 7;	// Near gale
	}
	
	if (metersPerSecond >= 17.2f && metersPerSecond < 20.8f) {
		return 8;	// Gale
	}
	
	if (metersPerSecond >= 20.8f && metersPerSecond < 24.5f) {
		return 9;	// Strong gale
	}
	
	if (metersPerSecond >= 24.5f && metersPerSecond < 28.5f) {
		return 10;	// Storm
	}
	
	if (metersPerSecond >= 28.5f && metersPerSecond < 32.7f) {
		return 11;	// Violent storm
	}
	
	return 12;	// Hurricane
	
}


static void UpdateDisplay(float frequency, float metersPerSecond, int force)
{	
	char line1[32];
	char line2[32];
		
		float speed = 0.0f;
		char* units = (char*)0;

		switch (DisplayMode_) {
			
			case DISPLAY_MODE_MPS:
				speed = metersPerSecond;
				units = "m/s";
				break;
			
			case DISPLAY_MODE_MPH:
				speed = metersPerSecond * CONVERSION_MPH;
				units = "mph";
				break;
			
			case DISPLAY_MODE_KNOTS:
				speed = metersPerSecond * CONVERSION_KNOTS;
				units = "kn";
				break;
		}
	
		char speedString[8];
		dtoa(speed, speedString, 1);
		strncat(speedString, units, strlen(units));
		
		strcpy(line1, speedString);
		strncat(line1, " ", 1);
		dtoa(frequency, &line1[strlen(line1)], 3);
		strncat(line1, "Hz", 2);

		strcpy(line2, windDescriptions_[force]);
		strncat(line2, "(", 1);
		itoa(force, &line2[strlen(line2)]);
		strncat(line2, ")", 1);
				
		pad(line1, ' ', 17);
		pad(line2, ' ', 17);
		
		LCD_PutString(&display_, line1, 0, 0);
		LCD_PutString(&display_, line2, 1, 0);
	
}


int Run(DeviceConfig_t* pConfig)
{
	
	if (!InitHardware(pConfig)) {
		for (;;);
	}
	
	LCD_RawClearDisplay(&display_.raw);

	for (;;) {
	
		SysTick_Wait10ms(10);
	
		Switch_Poll(&button_);
		if (Switch_HasClosed(button_)) {
			// Cycle through the display modes upon each button press.
			DisplayMode_ = (DisplayMode_ + 1) % DISPLAY_MODES;
		}
		
		float frequency = FrequencyTimer_GetFrequency(&freqTimer_);
		float metersPerSecond = 0.0f;
		if (frequency > 0.0f) {
			metersPerSecond = (frequency * pConfig->anemometer.slope) + pConfig->anemometer.offset;
		}
		int force = GetWindForce(metersPerSecond, pConfig->anemometer.offset);
		
		UpdateDisplay(frequency, metersPerSecond, force);
		
	}
	
}
