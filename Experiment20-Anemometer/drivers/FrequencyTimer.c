
#include "HAL.h"
#include "FrequencyTimer.h"
	
static void TimerEventCallback(TimerBlock_t block, const TimerEventArgs_t* args)
{
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)args->callbackData;
	
	if (args->eventType == TIMER_TIMEOUT_EVENT) {
		freqTimer->timeoutCounter--;
	}
	else if (args->eventType == TIMER_EDGE_EVENT) {
		uint32_t currentEdge = (freqTimer->timeoutCounter << 24) | args->eventData.timerCount;
		freqTimer->lastInterval = freqTimer->previousEdge - currentEdge;
		freqTimer->previousEdge = currentEdge;
	}

}

static void ResetTimer(FrequencyTimer_t* freqTimer)
{
	freqTimer->timeoutCounter = 255;
	freqTimer->previousEdge = 0;
	freqTimer->lastInterval = 0;
	freqTimer->pulseStarted = false;
}

float FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer)
{
	
	float frequency = 0.0f;
	
	// Copy the instantaneous edge interval into a local variable before calculating the frequency.
	// The disassembly shows that copying happens via an LDR instruction from the freqTimer field
	// address to a register (r0). Since the current machine instruction is finished before an
	// interrupt can occur, the LDR instruction will complete, and the frequency will be calculated
	// based on the last interval value stored in the register, even if a timer interrupt occurs before
	// the calculation completes.  In other words, a race condition should not be an issue for concern here.
	uint32_t edgeInterval = freqTimer->lastInterval;
	
	// A non-zero interval means that an edge has been detected after timer reset.  It's important that  
	// this function be called frequently enough to avoid timer wrap-around.  Otherwise, intervals equal
	// or greater than 2^32 will become misinterpreted.
	if (freqTimer->lastInterval) {
	
		// A non-zero interval means that an edge has been detected, but the frequency cannot be
		// calculated until there has been at least one previous edge detected.		
		if (freqTimer->pulseStarted) {
			
			// If the interval exceeds the maximum interval (minimum frequency), then an edge has
			// not occurred for a "very long" time.  It will mean there is no signal input, so reset and
			// wait for a new pulse train to start.
			if (edgeInterval > freqTimer->maxInterval) {
				ResetTimer(freqTimer);
			}
			else {
				// This calulates the instantaneous frequency based on the last recorded edge interval.
				frequency = (float)PLL_BusClockFreq / (float)edgeInterval;
			}
		}

		// At least one edge has occurred, so consider a pulse train to have started.
		freqTimer->pulseStarted = true;
	}
	
	return frequency;	
}


int FrequencyTimer_Enable(TimerBlock_t timer, const PinDef_t* pin, uint32_t maxInterval, uint8_t priority, FrequencyTimer_t* freqTimer)
{
	// Initialize the frequency timer data.
	freqTimer->maxInterval = maxInterval;
	ResetTimer(freqTimer);
	
	// Initialize the hardware timer for edge timer mode.
	TimerIRQConfig_t irqConfig = {priority, TimerEventCallback, (void*)freqTimer};
	if (Timer_Init(timer, TIMER_EDGE_TIMER, &irqConfig, pin)) {
		return -1;
	}
	
	// Start the hardware timer...
	Timer_StartEdgeTimer(timer);
	
	return 0;
}

