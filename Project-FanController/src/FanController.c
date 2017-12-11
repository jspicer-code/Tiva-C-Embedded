
#include <assert.h>
#include "FanController.h"
#include "HAL.h"
#include "Display.h"
#include "Thermistor.h"
#include "TemperatureSettings.h"
#include "Console.h"


// At 80MHz, this is the number of bus cycles for a 25kHz PWM frequency.
#define PWM_PERIOD		3200

// The number of times an ADC channel should be read before using the value.
#define MAX_ADC_SAMPLES 	150

// Pointers to the IO configuration and temperature settings, 
//	used by multiple functions.
static FanController_IOConfig_t* pIOConfig_;
static TemperatureSettings_t controlSettings_;

// Holds the bit-band alias address of the display switches.
static volatile uint32_t* pTempDisplaySwitch_;
static volatile uint32_t* pTachDisplaySwitch_;

// Holds the bit-band alias address of the fan mode switches.
static volatile uint32_t* pManualModeSwitch_;
static volatile uint32_t* pAutoModeSwitch_;

// Holds the bit-band address of the fan relay.
static volatile uint32_t* pFanRelay_;

// Holds the bit-band address of the heartbead LED.
static volatile uint32_t* pHeartbeatLED_;

// Holds the current RPM value.
static uint32_t tachRPM_;
		
// Names for the fan modes.
typedef enum {
		FANMODE_OFF,
		FANMODE_MANUAL,
		FANMODE_TEMP
} FanMode_t;

// Names for the display modes.
typedef enum {
		DISPLAYMODE_TACH,
		DISPLAYMODE_SPEED,
		DISPLAYMODE_TEMP
} DisplayMode_t;


// Catch-all error handler.
void ErrHandler(void) 
{
		// Spin...
		while (1);
}
	
// This function is called back by the Receive (Rx) UART interrupt handler
//	when a new character has arrived on the serial port.
static void UartRxCallback(char c)
{
	Console_HandleInput(c, &controlSettings_);
}	
	
	
// This function is called back an interval timer every second.  It's purpose
//	is to read the input edge counter and calculated the RPM of the fan.
//	It also toggles the "heartbeat" LED.
static void RpmTimerCallback(void)
{

	// Read the accumulated pulse count.
	uint32_t tachPulseCount = Timer_ReadCounterValue(pIOConfig_->tachCounter);

	// The Tachometer outputs 2 pulses per revolution.  So, divide by 2 to get
	//	revolutions per seconds, then multiply by 60 secs. to approximate RPM.
	tachRPM_ = (tachPulseCount / 2) * 60;
	
	// Reset the timer to zero and start over.
	Timer_ResetInputCounter(pIOConfig_->tachCounter);
	
	// Toggle the red onboard LED.
	*pHeartbeatLED_ = !(*pHeartbeatLED_);
	
}	

// Initialize the temperature control settings, reading them from Flash.
void InitTemperatureSettings(void)
{
	// Read the stored control settings from Flash.
	int wordCount = sizeof(TemperatureSettings_t) / 4;
	Flash_Read(&controlSettings_, wordCount); 
	
	// If the data in storage does not have the magic signature for the ControlSettings
	//	structure, then intialize and store with default values.
	if (controlSettings_.magic[0] != 'F' ||
		controlSettings_.magic[1] != 'A' ||
		controlSettings_.magic[2] != 'N' ||
		controlSettings_.magic[3] != 'C') {
			
		// Initialize with default settings.
		controlSettings_.scale = THERM_FAHRENHEIT;
		controlSettings_.calibrationOffset = 0;
		controlSettings_.lowTemp = 50;
		controlSettings_.highTemp = 100;
			
		controlSettings_.magic[0] = 'F';
		controlSettings_.magic[1] = 'A';
		controlSettings_.magic[2] = 'N';
		controlSettings_.magic[3] = 'C';			
		
		// Write the default settings to Flash.
		Flash_Write(&controlSettings_, wordCount);	
	}
	
}

// This function takes the IOConfig structure and initialize all hardware
//	and service modules used by this FanController.
static int InitHardware(FanController_IOConfig_t* pIOConfig)
{
	// If the pConfig pointer wasn't set by the caller, then abort.
	if (!pIOConfig) {
		return -1;
	}
	
	// Disable interrupts globally while configuring the hardware.
	__disable_irq();
	
	// Enable the PLL for 80MHz.
	PLL_Init80MHz();

	// Enable digital Inputs
	GPIO_EnableDI(pIOConfig->tempDisplaySwitch.port, pIOConfig->tempDisplaySwitch.pin, PULL_UP);
	GPIO_EnableDI(pIOConfig->tachDisplaySwitch.port, pIOConfig->tachDisplaySwitch.pin, PULL_UP);
	GPIO_EnableDI(pIOConfig->manualModeSwitch.port, pIOConfig->manualModeSwitch.pin, PULL_UP);
	GPIO_EnableDI(pIOConfig->tempModeSwitch.port, pIOConfig->tempModeSwitch.pin, PULL_UP);

	// Enable digital Outputs
	GPIO_EnableDO(pIOConfig->heartBeatLED.port, pIOConfig->heartBeatLED.pin, DRIVE_2MA, PULL_DOWN);
	GPIO_EnableDO(pIOConfig->fanRelay.port, pIOConfig->fanRelay.pin, DRIVE_2MA, PULL_DOWN);

	// Enable ADC Speed Pot
	ADC_Enable(pIOConfig->speedPot.module, pIOConfig->speedPot.channel);
	
	// Enable ADC Thermistor
	ADC_Enable(pIOConfig->thermistor.module, pIOConfig->thermistor.channel);
	
	// Enable the PWM output. The PWM period is calculated to be the number of ticks required
	//	to achieve a frequency of 25kHz, which is the nominal frequency for the fan.
	PWM_Enable(pIOConfig->pwm.module, pIOConfig->pwm.channel, PWM_PERIOD, PWM_PERIOD / 2);
	
	// Enable the RPM timer.  At 80MHz, there are 80 million system ticks in one second.
	//	Second highest interrupt priority.  The Display will take the highest.
	Timer_EnableTimerPeriodic(pIOConfig->rpmTimer, 80000000, 2, RpmTimerCallback);
	
	// Enable and reset the tach pulse input counter.
	Timer_EnableInputCounter(pIOConfig->tachCounter);
	Timer_ResetInputCounter(pIOConfig->tachCounter);
	
	// Enable the UART and configure it with an Rx interrupt.  Lowest priority interrupt.
	UART_Enable(pIOConfig->uart, 9600);
	UART_EnableRxInterrupt(pIOConfig->uart, 7, UartRxCallback);
	
	// Initialize the free-running timer.
	SysTick_Init();
	
	// Enable the Flash module.
	Flash_Enable();
	
	// Initialize the display module.
	Display_Initialize(pIOConfig->displaySSI, pIOConfig->displayTimer);
	
	// Initialize the console.
	Console_Init(pIOConfig->uart);
	
	// Store the bit-band addresses for the digital IO.
	pTempDisplaySwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->tempDisplaySwitch);
	pTachDisplaySwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->tachDisplaySwitch);	
	pManualModeSwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->manualModeSwitch);
	pAutoModeSwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->tempModeSwitch);	
	pFanRelay_ = GPIO_GetBitBandIOAddress(pIOConfig->fanRelay);	
	pHeartbeatLED_ = GPIO_GetBitBandIOAddress(pIOConfig->heartBeatLED);
		
	// Enable interrupts globally.
	__enable_irq();
	
	return 0;
}


// Reads and returns the ADC value associated with the manual speed potentiometer.
static int ReadSpeedPot(void)
{
	int adcSpeed = 0;
	
	for (int j = 0; j < MAX_ADC_SAMPLES; j++) {
		adcSpeed += ADC_Sample(pIOConfig_->speedPot.module);		
	}
	
	// Trim off LSBs and scale into range to get a more stable value.	
	adcSpeed /= MAX_ADC_SAMPLES;
	adcSpeed  &= 0xFFFFFFF0;
	adcSpeed  = (float)adcSpeed * 1.00368f;
	
	return adcSpeed;
}

// Reads and returns the ADC value associated with the temperature sensor.
static int ReadTemperature(void)
{
	const int MAX_READINGS = 10;
	
	static int count = -1;
	static int last = 0;	
	static int stable = 0;

	int adcTherm = 0;

	// Sample the ADC multiple times and take the average.
	for (int i = 0; i < MAX_ADC_SAMPLES; i++) {
		adcTherm += ADC_Sample(pIOConfig_->thermistor.module);		
	}
	
	adcTherm /= MAX_ADC_SAMPLES;
	
	// Convert ADC output to temperature (based on the configured scale) and add to it the calibration offset.
	int temperature = Therm_GetTemperature(adcTherm, controlSettings_.scale) + controlSettings_.calibrationOffset;	
		
	// First time initialization.
	if (count == -1) {
		last = temperature;
		stable = temperature;
		count = 0;
	}
	
	// Count how many times this temperature has been the same.
	if (temperature == last) {
		count++;
	}
	else {
		count = 0;
	}

	// If the current temperature has been the same for the last N readings,
	//	then proclaim it the stable temperature.
	if (count >= MAX_READINGS) {
		stable = temperature;
		count = 0;
	}

	last = temperature;
	
	return stable;
	
}

// Checks the state of the fan mode switches and returns the mode.
static FanMode_t GetFanMode(void)
{
	if (!(*pAutoModeSwitch_)) {
		return FANMODE_TEMP;
	}
	else if (!(*pManualModeSwitch_)) {
		return FANMODE_MANUAL;
	}
	
	return FANMODE_OFF;
	
}

// Checks the state of the display mode switches and returns the mode.
static DisplayMode_t GetDisplayMode(void)
{
	// Get the display based on the display mode switches.
	if (!(*pTempDisplaySwitch_)) {
		return DISPLAYMODE_TEMP;	
	}
	else if (!(*pTachDisplaySwitch_)) {
		return DISPLAYMODE_TACH;
	}	

	return DISPLAYMODE_SPEED;
}

// Turns on or off the fan and sets the PWM duty cycle.  The on/off state
//	and speed is determined by the mode of the fan which also depends on the
//	speed pot and temperature.
static int SetFanSpeed(int speedPot, int temperature)
{
	
	// percentage is equivalent to the duty cycle.
	float percentage = 0;
	
	// Get the fan mode from the mode switches.
	FanMode_t mode = GetFanMode();
	
	switch (mode) {
	
		case FANMODE_TEMP:
	
			// Turn on the fan if the temperature is EQUAL TO or ABOVE the lower limit,
			//	and map its speed into range with the high limit between 1-100%.
			if (temperature >= controlSettings_.lowTemp) {
				
				float range = (float)(controlSettings_.highTemp - controlSettings_.lowTemp);				
				
				// If the range is zero, then this is thermostatic contol - full speed.
				//	If the range is greater than zero, then use a linear speed range.
				//	Otherwise, if high is less than low, the fan stays off.
				if (range == 0.0f) {
					percentage = 1.0;
				}
				else if (range > 0.0f) {
					percentage = 0.01f + (0.99f *((float)(temperature - controlSettings_.lowTemp) / range));
				}			
			}
			break;
			
		case FANMODE_MANUAL:
			
			// Convert the ADC sample to a percentage of its maximum range.
			// 	Map the percentage into 1-100%.
			percentage  = 0.01f + (speedPot / 4096.0f);
			break;

		case FANMODE_OFF:
		default:
			// Fan is off.
			break;
	}

	if (percentage <= 0.0f) {
		
		// Turn the fan OFF.
		*pFanRelay_ = 0;
	
	}
	else {
				
		// Don't allow the duty cycle to go to 100% or the fan speed drops, 
		//	apparently unable to find an input pulse.
		if (percentage >= 1.0f) {
			percentage = 0.999f;
		}
	
		// Apply the percentage to the PWM period to obtain the duty cycle.
		uint32_t duty = (uint32_t)(percentage * (float)PWM_PERIOD);

		// Set the duty cycle
		PWM_SetDuty(pIOConfig_->pwm.module, pIOConfig_->pwm.channel, duty);
	
		// Turn the fan ON.
		*pFanRelay_ = 1;
	
	}
		
	// Return an integer speed value between 0-100.
	return (int)((percentage * 100.0f) + 0.5f);
	
}

// The function checks the current display mode and updates the Display
//	module with a value to display.
static void UpdateDisplay(int fanSpeed, int temperature)
{
	
	int displayValue;
	DisplayMode_t displayMode = GetDisplayMode();
	
	switch (displayMode) {
		
		case DISPLAYMODE_TEMP:
			displayValue = temperature;
			break;
		
		case DISPLAYMODE_TACH:
			displayValue = tachRPM_;
			break;
		
		case DISPLAYMODE_SPEED:
		default:
			displayValue = fanSpeed;
			break;
	
	}
	
	// Temperature could become a negative number, which isn't supported by the display,
	//	so force it to be positive. 
	if (displayValue < 0) {
		displayValue *= -1;
	}
	
	// Update the display.
	Display_Update(displayValue);			
	
}


// Scan function reads the analog and switch inputs, calculates and sets the fan speed,
//	and updates the display.
static void Scan(void)
{

	// Get the value (0-4095) of the manual speed potentiometer.
	int speedPot = ReadSpeedPot();
	
	// Get the current stable temperature. 
	int temperature = ReadTemperature();

	// Calculate and set the fan speed.
	int fanSpeed = SetFanSpeed(speedPot, temperature); 
		
	// Update the display.
	UpdateDisplay(fanSpeed, temperature);
	
}

//----------------------- FanController_Run --------------------------
// Runs the FanController.  This is the main entry point.  Blocks and 
//   doesn't return.  Should be called by the main() function upon startup.
// Inputs:  pIOConfig - a pointer to the IO configuration that the 
//   FanController should use.
// Outputs:  none
void FanController_Run(FanController_IOConfig_t* pIOConfig)
{
	
	// Initialize the device IO.  If this fails then go no further.
	if (InitHardware(pIOConfig)) {
		ErrHandler();
	}

	// Save a pointer to the IO config.  It will be used again inside the callback functions.
	pIOConfig_ = pIOConfig;

	// Read the temperature control settings from Flash.
	InitTemperatureSettings();
		
	while (1) 
	{			

		// Read inputs, process, and write outputs...
		Scan();

		// Sleep approximately 100ms...
		SysTick_Wait10ms(10);

	}
	
}
