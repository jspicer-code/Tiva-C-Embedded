// File:  HAL_Timer.h
// Author: JSpicer
// Date:  10/06/17
// Purpose: Timer utilities
// Hardware:  TM4C123 Tiva board

#ifndef TIMER_HAL_H
#define TIMER_HAL_H

#include <stdint.h>

#define Timer_Wait10ms(timer, count) Timer_Wait(timer, count, PLL_NumTicksPer10ms);
#define Timer_Wait10us(timer, count) Timer_Wait(timer, count, PLL_NumTicksPer10us);
#define Timer_Wait100ns(timer, count) Timer_Wait(timer, count, PLL_NumTicksPer100ns);

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
#if (halCONFIG_1294 == 1)
	TIMER6 = 6,
	TIMER7 = 7,
#endif
} TimerBlock_t;

typedef enum {
	TIMER_ONESHOT,
	TIMER_PERIODIC
} TimerMode_t;


//------------------ Timer_Init --------------------------
// Initializes a timer block for periodic or one shot (count down) mode.  
//	Optionally, each time the timer expires it invokes the callback function
//	at restarts.
// Inputs:  block - the hardware time block to use.
//					mode - 0: one shot, 1: periodic
//          priority - interrupt priority.
//          callback - function to callback when the timer expires.
// Outputs:  none.
int Timer_Init(TimerBlock_t block, TimerMode_t mode, uint8_t priority, PFN_TimerCallback callback);

int Timer_Start(TimerBlock_t block, uint32_t interval);

void Timer_Stop(TimerBlock_t block);
	
//------------------ Timer_EnableInputCounter --------------------------
// Initializes a timer block for input edge counter mode.
//	The timer keeps an accumulator of the number of times
//	a rising edge pulse is detected on its associated input.  
// Inputs:  block - the hardware timer block to use.
//					pinDef - pin mux/pin assignment
// Outputs:  none.
int Timer_EnableInputCounter(TimerBlock_t block, PinDef_t pinDef);

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

//----------------------- Timer_Wait --------------------------
// Waits (blocks) for a given number of bus cycles.
// Inputs:  block - the name of the timer blocks
//					count - the number of times the interval must ellapse
//				  interval - the timer preset (bus cycles)
// Outputs:  none.
void Timer_Wait(TimerBlock_t block, uint32_t count, uint32_t interval);

#endif
