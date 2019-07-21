
#include "HAL.h"
#include "FrequencyTimer.h"
	

static void TimerEventCallback(TimerBlock_t block, const TimerEventArgs_t* args)
{
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)args->callbackData;
	uint32_t currentCount = (freqTimer->cycle << 24) | args->eventData.timerCount;
	uint32_t interval = freqTimer->previousCount - currentCount;
	
	if (args->eventType == TIMER_TIMEOUT_EVENT) {		
		if (interval <= freqTimer->maxInterval) {
			freqTimer->cycle--;
		}
		else {
			// If the interval exceeds the maximum interval (minimum frequency), then an edge has not occurred
			// for a "very long" time.  It will mean there is no signal input, so reset the timer.
			freqTimer->cycle = 255;
			freqTimer->previousCount = 0;
			freqTimer->lastInterval = 0;
			freqTimer->pulseStarted = false;
		}
	}
	else if (args->eventType == TIMER_EDGE_EVENT) { 
		if (freqTimer->pulseStarted){
			freqTimer->lastInterval = interval;
		}
		else {
			freqTimer->pulseStarted = true;
		}
		freqTimer->previousCount = currentCount;
	}
	
}


double FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer)
{
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
			return (double)PLL_BusClockFreq / (double)lastInterval;
	}
	
	return 0.0;	
}


int FrequencyTimer_Enable(TimerBlock_t timer, const PinDef_t* pin, uint32_t maxInterval, uint8_t priority, FrequencyTimer_t* freqTimer)
{
	// Initialize timer state.
	freqTimer->maxInterval = maxInterval;
	freqTimer->cycle = 255;
	freqTimer->previousCount = 0;
	freqTimer->lastInterval = 0;
	freqTimer->pulseStarted = false;
	
	// Initialize the hardware timer for edge timer mode.
	TimerIRQConfig_t irqConfig = {priority, TimerEventCallback, (void*)freqTimer};
	if (Timer_Init(timer, TIMER_EDGE_TIMER, &irqConfig, pin)) {
		return -1;
	}
	
	// Start the hardware timer...
	Timer_StartEdgeTimer(timer);
	
	return 0;
}

