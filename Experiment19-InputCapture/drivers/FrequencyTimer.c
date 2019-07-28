#include "HAL.h"
#include "FrequencyTimer.h"
	
#define CYCLE_UNDETECTED 			0x00
#define CYCLE_STARTED					0x01
#define CYCLE_DETECTED				0x02

static void TimerEventCallback(TimerBlock_t block, const TimerEventArgs_t* args)
{
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)args->callbackData;
	uint32_t currentCount = (freqTimer->timeoutCounter << 24) | args->eventData.timerCount;
	uint32_t interval = freqTimer->previousCount - currentCount;
	
	bool disableTimer = false;
	if (args->eventType == TIMER_TIMEOUT_EVENT) {		

		freqTimer->timeoutCounter--;
		
		// If the interval exceeds the maximum interval (minimum frequency), then an edge has not occurred
		// for a "very long" time.  It will mean there is no signal input, so clear the last interval.
		if (interval > freqTimer->maxInterval) {
			freqTimer->lastInterval = 0;
			disableTimer = true;
		}
		
	}
	else if (args->eventType == TIMER_EDGE_EVENT) {

		if (freqTimer->cycleStatus == CYCLE_UNDETECTED) {
			freqTimer->cycleStatus = CYCLE_STARTED;
		}
		else {
			
			freqTimer->lastInterval = interval;
			freqTimer->cycleStatus = CYCLE_DETECTED;
			
			// If a cycle has been detected but the frequency is over a certain limit, stop the timer so
			// that continual reentry of the ISR does not cause CPU starvation.
			if (freqTimer->lastInterval < freqTimer->highFreqInterval) {
				disableTimer = true;
			}
		}
	
		freqTimer->previousCount = currentCount;
	}
		
	if (disableTimer) {
		Timer_DisableEdgeTimer(block);
	}
	
}

static void StartTimer(FrequencyTimer_t* freqTimer)
{
		freqTimer->timeoutCounter = 255;
		freqTimer->previousCount = 0;
		freqTimer->cycleStatus = CYCLE_UNDETECTED;
	
		Timer_EnableEdgeTimer(freqTimer->timer);
}

double FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer)
{
	double frequency = 0.0;
	
	// Copy the instantaneous edge interval into a local variable before calculating the frequency.
	// The disassembly shows that copying happens via an LDR instruction from the freqTimer field
	// address to a register (r0). Since the current machine instruction is finished before an
	// interrupt can occur, the LDR instruction will complete, and the frequency will be calculated
	// based on the last interval value stored in the register, even if a timer interrupt occurs before
	// the calculation completes.  In other words, a race condition should not be an issue for concern here.
	uint32_t lastInterval = freqTimer->lastInterval;
	
	// This calulates the instantaneous frequency based on the last recorded edge interval.
	// If the last interval is zero it means that two edges have not been detected.
	if (lastInterval) {
		frequency = (double)PLL_BusClockFreq / (double)lastInterval;		
	}
	
	// The timer will stop if the last interval exceeded the minimum or high frequency limit.
	if (!Timer_IsTimerEnabled(freqTimer->timer)) {	
		StartTimer(freqTimer);	
	}
	
	return frequency;	
}


int FrequencyTimer_Enable(TimerBlock_t timer, uint8_t priority, const PinDef_t* pin, float minFrequency, FrequencyTimer_t* freqTimer)
{
	// Initialize timer state.
	freqTimer->timer = timer;
	freqTimer->maxInterval = (float)PLL_BusClockFreq / minFrequency;
	freqTimer->highFreqInterval =  (float)PLL_BusClockFreq / 1000.0f; // 1 kHz
	
	// Initialize the hardware timer for edge timer mode.
	TimerIRQConfig_t irqConfig = {priority, TimerEventCallback, (void*)freqTimer};
	if (Timer_Init(timer, TIMER_EDGE_TIMER, &irqConfig, pin)) {
		return -1;
	}
	
	// Start the hardware timer...
	StartTimer(freqTimer);
	
	return 0;
}

