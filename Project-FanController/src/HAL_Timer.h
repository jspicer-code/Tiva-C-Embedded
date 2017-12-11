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

// Names of the timer blocks.
typedef enum {
	TIMER0 = 0,
	TIMER1 = 1,
	TIMER2 = 2,
	TIMER3 = 3,
	TIMER4 = 4,
	TIMER5 = 5,
} TimerBlock_t;

//------------------ Timer_EnableTimerPeriodic --------------------------
// Initializes a timer block for periodic (count down) mode.  
//	Each time the timer expires it invokes the callback function
//	at restarts.
// Inputs:  block - the hardware time block to use.
//          interval - the number of bus cycles per countdown interval.
//          priority - interrupt priority.
//          callback - function to callback when the timer expires.
// Outputs:  none.
int Timer_EnableTimerPeriodic(TimerBlock_t block, uint32_t interval, uint8_t priority, PFN_TimerCallback callback);


//------------------ Timer_EnableInputCounter --------------------------
// Initializes a timer block for input edge counter mode.
//	The timer keeps an accumulator of the number of times
//	a rising edge pulse is detected on its associated input.  
// Inputs:  block - the hardware timer block to use.
// Outputs:  none.
int Timer_EnableInputCounter(TimerBlock_t block);

//------------------ Timer_ResetInputCounter --------------------------
// Resets the accumulator of a timer configured for input edge counter mode.
// Inputs:  block - the hardware timer block to use.
// Outputs:  none.
void Timer_ResetInputCounter(TimerBlock_t block);

//------------------ Timer_ReadCounterValue --------------------------
// Reads the accumulator of a timer configured for input edge counter mode.
// Inputs:  block - the hardware timer block to use.
// Outputs:  accumlator value.
uint32_t Timer_ReadCounterValue(TimerBlock_t block);


#endif
