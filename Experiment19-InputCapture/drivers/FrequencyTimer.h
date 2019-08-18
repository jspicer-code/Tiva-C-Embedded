// File: FrequencyTimer.h
// Author: JSpicer
// Date:  07/28/19
// Purpose: Measures the frequency of a periodic pulse input.
// Hardware:  TM4C123/TM4C129 Tiva C Launchpad board

#ifndef FREQUENCY_TIMER_H
#define FREQUENCY_TIMER_H

#include "HAL.h"

typedef struct {
	TimerBlock_t 	timer;
	PinDef_t 			pin;
	uint8_t 			priority;
} FrequencyTimerConfig_t;

typedef struct FrequencyTimer FrequencyTimer_t;

// Enables the frequency timer for given time block, interrupt priority, I/O pin, and minimum frequency
FrequencyTimer_t* FrequencyTimer_Enable(const FrequencyTimerConfig_t* config);

// Polls for the most recently measured pulse frequency.
float FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer);

#endif
