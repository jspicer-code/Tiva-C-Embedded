
#include <assert.h>
#include "FanController.h"
#include "HAL.h"
#include "Display.h"
#include "Thermistor.h"

// At 80MHz, this is the number of bus cycles for a 25kHz PWM frequency.
#define PWM_PERIOD		3200

// Program states.
enum {
	STATE_STOPPED,
	STATE_SELECTION,
	STATE_SCAN
} appState_ = STATE_STOPPED;


static FanController_Config_t* pConfig_;

static volatile uint32_t* pTempDisplaySwitch_;
static volatile uint32_t* pTachDisplaySwitch_;

static volatile uint32_t* pManualModeSwitch_;
static volatile uint32_t* pAutoModeSwitch_;

static volatile uint32_t* pFanRelay_;
static volatile uint32_t* pHeartbeatLED_;

static uint32_t tachRPM_;
	
	
// Transmits the color selection menu over the UART.
static void PrintMenu(void)
{
	
	UART_WriteString(pConfig_->uart, "\n\r\n\r");		
	UART_WriteString(pConfig_->uart, "Configuration Menu:\n\r");
	UART_WriteString(pConfig_->uart, "1) Change Temperature Scale\n\r");
	UART_WriteString(pConfig_->uart, "2) Calibrate Temperature Probe\n\r");
	UART_WriteString(pConfig_->uart, "3) Set Temperature Control Range\n\r");
	UART_WriteString(pConfig_->uart, ">");	

}

void ErrHandler(void) 
{
		// Spin...
		while (1);
}

	
// This function is called back by the Receive (Rx) UART interrupt handler
//	when a new character has arrived on the serial port.
static void UartRxCallback(char c)
{
	// Change state so that the main program will know to display the configuration menu.
	if (appState_ == STATE_SCAN) {
		appState_ = STATE_SELECTION;
	}
}	
	
	
static void RpmTimerCallback(void)
{

	// Read the accumulated pulse count.
	uint32_t tachPulseCount = Timer_ReadCounterValue(pConfig_->tachCounter);

	// The Tachometer outputs 2 pulses per revolution.  So, divide by 2 to get
	//	revolutions per seconds, then multiply by 60 secs. to approximate RPM.
	tachRPM_ = (tachPulseCount / 2) * 60;
	
	// Reset the timer to zero and start over.
	Timer_ResetInputCounter(pConfig_->tachCounter);
	
	// Toggle the red onboard LED.
	*pHeartbeatLED_ = !(*pHeartbeatLED_);
	
}	


static int InitHardware(FanController_Config_t* pConfig)
{
	
	// If the pConfig pointer wasn't set by the caller, then abort.
	if (!pConfig) {
		return -1;
	}
	
	// Disable interrupts globally while configuring the hardware.
	__disable_irq();
	
	// Enable the PLL for 80MHz.
	PLL_Init80MHz();
	
	// Digital Inputs
	GPIO_EnableDI(pConfig->tempDisplaySwitch.port, pConfig->tempDisplaySwitch.pin, PULL_UP);
	GPIO_EnableDI(pConfig->tachDisplaySwitch.port, pConfig->tachDisplaySwitch.pin, PULL_UP);
	GPIO_EnableDI(pConfig->manualModeSwitch.port, pConfig->manualModeSwitch.pin, PULL_UP);
	GPIO_EnableDI(pConfig->autoModeSwitch.port, pConfig->autoModeSwitch.pin, PULL_UP);

	// Digital Outputs
	GPIO_EnableDO(pConfig->heartBeatLED.port, pConfig->heartBeatLED.pin, DRIVE_2MA, PULL_DOWN);
	GPIO_EnableDO(pConfig->fanRelay.port, pConfig->fanRelay.pin, DRIVE_2MA, PULL_DOWN);

	// ADC Speed Pot
	ADC_Enable(pConfig->speedPot.module, pConfig->speedPot.channel);
	
	// ADC Thermistor
	ADC_Enable(pConfig->thermistor.module, pConfig->thermistor.channel);
	
	// The PWM period is calculated to be the number of ticks to achieve
	//	a frequency of 25kHz.
	PWM_Enable(pConfig->pwm.module, pConfig->pwm.channel, PWM_PERIOD, PWM_PERIOD / 2);
	
	// At 80MHz, there are 80 million system ticks in one second.
	Timer_EnableTimerPeriodic(pConfig->rpmTimer, 80000000, 2, RpmTimerCallback);
	
	// Initialize the tach pulse input counter.
	Timer_InitInputCounter(pConfig->tachCounter);
	Timer_ResetInputCounter(pConfig->tachCounter);
	
	// Enable UART for Rx interrupt.
	UART_Init(pConfig->uart, 9600);
	UART_EnableRxInterrupt(pConfig->uart, 7, UartRxCallback);
	
	// Initialize the free-running timer.
	SysTick_Init();
	
	// Initialize the display.
	Display_Initialize(pConfig->displaySSI, pConfig->displayTimer);
	
	// Store the bit-band addresses for the digital IO.
	pTempDisplaySwitch_ = GPIO_GetBitBandIOAddress(pConfig->tempDisplaySwitch);
	pTachDisplaySwitch_ = GPIO_GetBitBandIOAddress(pConfig->tachDisplaySwitch);	
	pManualModeSwitch_ = GPIO_GetBitBandIOAddress(pConfig->manualModeSwitch);
	pAutoModeSwitch_ = GPIO_GetBitBandIOAddress(pConfig->autoModeSwitch);	
	pFanRelay_ = GPIO_GetBitBandIOAddress(pConfig->fanRelay);	
	pHeartbeatLED_ = GPIO_GetBitBandIOAddress(pConfig->heartBeatLED);
		
	// Enable interrupts globally.
	__enable_irq();
	
	return 0;
}


static void Scan(void)
{
	int adcSpeed = 0;
	int adcTherm = 0;
	float percentage;
	
	for (int j = 0; j < 50; j++) {
		adcSpeed += ADC_Sample(pConfig_->speedPot.module);		
	}
	
	// Trim off LSBs and scale into range to avoid jitter.	
	adcSpeed /= 50;
	adcSpeed  &= 0xFFFFFFF0;
	adcSpeed  = (float)adcSpeed * 1.00368f;
			
	// TODO:  test averaging the therm input.
	for (int i = 0; i < 50; i++) {
		adcTherm = ADC_Sample(pConfig_->thermistor.module);		
	}
		
	// Convert sample to temperature.
	int temperature = Therm_GetTemperature(adcTherm, THERM_FAHRENHEIT);	
	
	// Update the display based on the display mode switches.
	if (!(*pTempDisplaySwitch_)) {
		Display_Update(temperature);			
	}
	else if (!(*pTachDisplaySwitch_)) {
		Display_Update(tachRPM_);
	}	
	else {
		Display_Update(adcSpeed);
	}
	
	// Check the state of the fan mode switches to determine the fan's state (on/off) and duty cycle.
	if (!(*pAutoModeSwitch_)) {
		
		// TODO:  This range should be configurable from the menu.
		*pFanRelay_ = (temperature >= 50) ? 1 : 0;
		percentage = (float)(temperature - 50) / (float)(100 - 50);
	
	}
	else if (!(*pManualModeSwitch_)) {
		
		// Turn on the fan relay.
		*pFanRelay_ = 1;
		
		// Convert it to a percentage of its maximum range.
		percentage = (float)(adcSpeed / 4095.0f);
				
	}
	else {
		// Turn off the fan.
		*pFanRelay_ = 0;
		percentage = 0;
	}
	
	//	Don't allow the duty cycle to go to 100% or the fan speed drops, 
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
	PWM_SetDuty(pConfig_->pwm.module, pConfig_->pwm.channel, duty);
	
}


void FanController_Run(FanController_Config_t* pConfig)
{
	
	// If hardware initialization has fails then go no further.
	if (InitHardware(pConfig)) {
		ErrHandler();
	}
	
	// Save the pointer to the config.  It will be used inside the callback functions.
	pConfig_ = pConfig;
	
	appState_ = STATE_SCAN;
	
	while (1) 
	{			

		switch (appState_) {
			
			case STATE_SELECTION:
				PrintMenu();
				appState_ = STATE_SCAN;
				break;
			
			case STATE_SCAN:
				Scan();
				break;
		
		}
		
		// Wait approximately 100ms...
		SysTick_Wait10ms(10);

	}
	
}
