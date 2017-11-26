// File:  HAL_Timer.h
// Author: JSpicer
// Date:  10/06/17
// Purpose: Timer utilities
// Hardware:  TM4C123 Tiva board

#ifndef TIMER_HAL_H
#define TIMER_HAL_H

#include <stdint.h>

// Signature of the Timer callback function 
typedef void (*PFN_TimerCallback)(void);

typedef enum {
	TIMER0 = 0,
	TIMER1 = 1,
	TIMER2 = 2,
	TIMER3 = 3,
	TIMER4 = 4,
	TIMER5 = 5,
} TimerBlock_t;

int Timer_EnableTimerPeriodic(TimerBlock_t block, uint32_t interval, uint8_t priority, PFN_TimerCallback callback);
int Timer_EnableInputCounter(TimerBlock_t block);
void Timer_ResetInputCounter(TimerBlock_t block);
uint32_t Timer_ReadCounterValue(TimerBlock_t block);


#endif
