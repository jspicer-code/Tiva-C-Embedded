
#include <assert.h>
#include "FanController.h"
#include "HAL.h"
#include "Display.h"
#include "Thermistor.h"
#include "ControlSettings.h"
#include "Console.h"

// At 80MHz, this is the number of bus cycles for a 25kHz PWM frequency.
#define PWM_PERIOD		3200

static FanController_IOConfig_t* pIOConfig_;
static ControlSettings_t controlSettings_;

static volatile uint32_t* pTempDisplaySwitch_;
static volatile uint32_t* pTachDisplaySwitch_;

static volatile uint32_t* pManualModeSwitch_;
static volatile uint32_t* pAutoModeSwitch_;

static volatile uint32_t* pFanRelay_;
static volatile uint32_t* pHeartbeatLED_;

static uint32_t tachRPM_;
		
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


void InitControlSettings(void)
{
	controlSettings_.scale = THERM_FAHRENHEIT;
	controlSettings_.calibrationOffset = 0;
	controlSettings_.lowTemp = 50;
	controlSettings_.highTemp = 100;
}

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
	GPIO_EnableDI(pIOConfig->autoModeSwitch.port, pIOConfig->autoModeSwitch.pin, PULL_UP);

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
	Timer_EnableTimerPeriodic(pIOConfig->rpmTimer, 80000000, 2, RpmTimerCallback);
	
	// Enable and reset the tach pulse input counter.
	Timer_EnableInputCounter(pIOConfig->tachCounter);
	Timer_ResetInputCounter(pIOConfig->tachCounter);
	
	// Enable the UART and configure it with an Rx interrupt.
	UART_Enable(pIOConfig->uart, 9600);
	UART_EnableRxInterrupt(pIOConfig->uart, 7, UartRxCallback);
	
	// Initialize the free-running timer.
	SysTick_Init();
	
	// Initialize the display module.
	Display_Initialize(pIOConfig->displaySSI, pIOConfig->displayTimer);
	
	// Initialize the console.
	Console_Init(pIOConfig->uart);
	
	// Store the bit-band addresses for the digital IO.
	pTempDisplaySwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->tempDisplaySwitch);
	pTachDisplaySwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->tachDisplaySwitch);	
	pManualModeSwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->manualModeSwitch);
	pAutoModeSwitch_ = GPIO_GetBitBandIOAddress(pIOConfig->autoModeSwitch);	
	pFanRelay_ = GPIO_GetBitBandIOAddress(pIOConfig->fanRelay);	
	pHeartbeatLED_ = GPIO_GetBitBandIOAddress(pIOConfig->heartBeatLED);
		
	// Enable interrupts globally.
	__enable_irq();
	
	return 0;
}


static void Scan(void)
{

	int adcSpeed = 0;
	int adcTherm = 0;
	float percentage;
	const int MAX_ADC_SAMPLES = 50;
	
	for (int j = 0; j < MAX_ADC_SAMPLES; j++) {
		adcSpeed += ADC_Sample(pIOConfig_->speedPot.module);		
	}
	
	// Trim off LSBs and scale into range to avoid jitter.	
	adcSpeed /= MAX_ADC_SAMPLES;
	adcSpeed  &= 0xFFFFFFF0;
	adcSpeed  = (float)adcSpeed * 1.00368f;
			
	// TODO:  test averaging the therm input.
	for (int i = 0; i < MAX_ADC_SAMPLES; i++) {
		adcTherm += ADC_Sample(pIOConfig_->thermistor.module);		
	}
	adcTherm /= MAX_ADC_SAMPLES;	
	
	// Convert sample to temperature (based on the configured scale) and add to it the calibration offset.
	int temperature = Therm_GetTemperature(adcTherm, controlSettings_.scale) + controlSettings_.calibrationOffset;	
	
	// Check the state of the fan mode switches to determine the fan's state (on/off) and duty cycle.
	if (!(*pAutoModeSwitch_)) {
		
		// Turn on the fan if the temperature is above the mininum control value and scale its speed into range.
		*pFanRelay_ = (temperature >= controlSettings_.lowTemp) ? 1 : 0;
		percentage = (float)(temperature - controlSettings_.lowTemp) / (float)(controlSettings_.highTemp - controlSettings_.lowTemp);
	
	}
	else if (!(*pManualModeSwitch_)) {
		
		// Turn on the fan relay and convert the ADC sample to a percentage of its maximum range.
		*pFanRelay_ = 1;
		percentage = (float)(adcSpeed / 4095.0f);
				
	}
	else {
		// Turn off the fan.
		*pFanRelay_ = 0;
		percentage = 0;
	}
	
	// Don't allow the duty cycle to go to 100% or the fan speed drops, 
	//	apparently unable to find an input pulse.
	if (percentage >= 1.0f) {
		percentage = 0.999f;
	}
	else if (percentage <= 0.0f) {
		percentage = 0.001f;
	}
	
	// Apply the percentage to the PWM period to obtain the duty cycle.
	uint32_t duty = (uint32_t)(percentage * (float)PWM_PERIOD);

	// Set the duty cycle
	PWM_SetDuty(pIOConfig_->pwm.module, pIOConfig_->pwm.channel, duty);
	
	// Update the display based on the display mode switches.
	if (!(*pTempDisplaySwitch_)) {
		Display_Update(temperature);			
	}
	else if (!(*pTachDisplaySwitch_)) {
		Display_Update(tachRPM_);
	}	
	else {
		Display_Update((percentage * 100.0f) + 0.5f);
	}
	
}

void FanController_Run(FanController_IOConfig_t* pIOConfig)
{

	// Initialize the temperature control settings.
	InitControlSettings();
	
	// Initialize the device IO.  If this fails then go no further.
	if (InitHardware(pIOConfig)) {
		ErrHandler();
	}
	
	// Save a pointer to the IO config.  It will be used again inside the callback functions.
	pIOConfig_ = pIOConfig;
		
	while (1) 
	{			

		// Read inputs, process, and write outputs...
		Scan();

		// Sleep approximately 100ms...
		SysTick_Wait10ms(10);

	}
	
}
