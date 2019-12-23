// File: CaptureTimer.h
// Author: JSpicer
// Date:  07/28/19
// Purpose: Measures the frequency and duty cycle of a periodic pulse input.
// Hardware:  TM4C123/TM4C129 Tiva C Launchpad board

#ifndef CAPTURE_TIMER_H
#define CAPTURE_TIMER_H

#include "HAL.h"

typedef enum {
	CAPTIMER_PULSE_VALID = 0,
	CAPTIMER_PULSE_INVALID,
	CAPTIMER_PULSE_NOSIGNAL
} CaptureTimer_PulseStatus_t;

typedef struct {
	TimerBlock_t 	timer;
	PinDef_t 			timerPin;
	PinDef_t			levelPin;
	uint8_t 			irqPriority;
	bool					measureDutyCycle;
} CaptureTimer_Config_t;

typedef struct {
	float frequency;
	float dutyCycle;
} CaptureTimer_PulseInfo_t;

typedef struct CaptureTimer CaptureTimer_t;

// Initializes and enables the capture timer for a given timer block
CaptureTimer_t* CaptureTimer_Init(const CaptureTimer_Config_t* config);

// Terminates the capture timer, disabling it and freeing memory.
void CaptureTimer_Terminate(CaptureTimer_t* capTimer);

// Polls for the average pulse frequency and duty cycle.
CaptureTimer_PulseStatus_t CaptureTimer_GetPulse(CaptureTimer_t* capTimer, CaptureTimer_PulseInfo_t* pulse);

#endif
