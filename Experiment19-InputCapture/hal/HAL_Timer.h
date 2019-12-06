// File:  HAL_Timer.h
// Author: JSpicer
// Date:  10/06/17
// Purpose: Timer utilities
// Hardware:  TM4C123 Tiva board

#ifndef TIMER_HAL_H
#define TIMER_HAL_H

#include <stdint.h>
#include "HAL.h"

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
	TIMERA = 0,
	TIMERB = 1,
} TimerSubTimer_t;

// Types of timers.
typedef enum {
	TIMER_ONESHOT,
	TIMER_PERIODIC,
} TimerIntervalMode_t;

// Types of timer events (edge events).
typedef enum {
	TIMER_EVENT_RISING_EDGE = 0,
	TIMER_EVENT_FALLING_EDGE = 1,
	TIMER_EVENT_BOTH_EDGES = 3
} TimerEventType_t;


// This structure represents the registers associated with a timer block.
//	It will be overlayed on top of IO memory so that the structure fields
//	map to the registers.  (See the datasheet for field/register descriptions).
typedef struct {
	uint32_t  CFG;
	uint32_t  TAMR;
	uint32_t  TBMR;
	uint32_t  CTL;
	uint32_t  SYNC;
	uint32_t  RESERVED;
	uint32_t  IMR;
	uint32_t  RIS;
	uint32_t  MIS;
	uint32_t  ICR;
	uint32_t  TAILR;
	uint32_t  TBILR;
	uint32_t  TAMATCHR;
	uint32_t  TBMATCHR;
	uint32_t  TAPR;
	uint32_t  TBPR;
	uint32_t  TAPMR;
	uint32_t  TBPMR;
	uint32_t  TAR;
	uint32_t  TBR;
	uint32_t  TAV;
	uint32_t  TBV;
	uint32_t  RTCPD;
	uint32_t  TAPS;
	uint32_t  TBPS;
	
#if (halCONFIG_1294 == 1)
	uint32_t  RESERVED1[2];
	uint32_t  DMAEV;
	uint32_t  ADCEV;
	uint32_t  RESERVED2[979];
	uint32_t  PP;
	uint32_t  RESERVED3;
	uint32_t  CC;
#else // TM4C123
	uint32_t  TAPV;		// 32/64-bit Wide GPTM
	uint32_t  TBPV;		// 32/64-bit Wide GPTM
	uint32_t  RESERVED1[981];
	uint32_t  PP;
#endif

} TimerRegs_t;


typedef struct {
	uint8_t priority;
	void* callbackData;
} TimerIRQConfig_t;

// This structure holds information for the interrupt handler.
typedef struct {
	volatile TimerRegs_t* timerRegs;	
	void* timerState;
} TimerISRData_t;


//------------------ Timer_Init --------------------------
// Initializes a timer block for periodic or one shot (count down) mode.  
//	Optionally, each time the timer expires it invokes the callback function
//	at restarts.
// Inputs:  block - the hardware time block to use.
//					mode - 0: one shot, 1: periodic
//          priority - interrupt priority.
//          callback - function to callback when the timer expires.
// Outputs:  none.
//int Timer_Init(TimerBlock_t block, TimerMode_t mode, const TimerIRQConfig_t* irqConfig, const PinDef_t* pinConfig);

bool Timer_IsTimerEnabled(TimerBlock_t block);

volatile TimerRegs_t* Timer_GetRegisters(TimerBlock_t block);

#if (halUSE_INTERVAL_TIMERS == 1)

int Timer_InitIntervalTimer(TimerBlock_t block, TimerIntervalMode_t mode, const TimerIRQConfig_t* irqConfig, const PinDef_t* pinConfig);

int Timer_StartInterval(TimerBlock_t block, uint32_t interval);

//----------------------- Timer_Wait --------------------------
// Waits (blocks) for a given number of bus cycles.
// Inputs:  block - the name of the timer blocks
//					count - the number of times the interval must ellapse
//				  interval - the timer preset (bus cycles)
// Outputs:  none.
void Timer_Wait(TimerBlock_t block, uint32_t count, uint32_t interval);

#define Timer_Wait10ms(timer, count) Timer_Wait(timer, count, PLL_NumTicksPer10ms);
#define Timer_Wait10us(timer, count) Timer_Wait(timer, count, PLL_NumTicksPer10us);
#define Timer_Wait100ns(timer, count) Timer_Wait(timer, count, PLL_NumTicksPer100ns);

#endif


#if (halUSE_EDGE_TIME_TIMERS == 1) 

int Timer_InitEdgeTimeTimer(TimerBlock_t block, TimerEventType_t eventType, const PinDef_t* pinConfig, const TimerIRQConfig_t* irqConfig);
void Timer_EnableEdgeTimeTimer(TimerBlock_t block);
void Timer_DisableEdgeTimeTimer(TimerBlock_t block);

#endif

#if (halUSE_EDGE_COUNT_TIMERS == 1) 
//------------------ Timer_EnableInputCounter --------------------------
// Initializes a timer block for input edge counter mode.
//	The timer keeps an accumulator of the number of times
//	a rising edge pulse is detected on its associated input.  
// Inputs:  block - the hardware timer block to use.
//					pinDef - pin mux/pin assignment
// Outputs:  none.
int Timer_EnableEdgeCountTimer(TimerBlock_t block);

void Timer_DisableEdgeCountTimer(TimerBlock_t block);

//------------------ Timer_ResetInputCounter --------------------------
// Resets the accumulator of a timer configured for input edge counter mode.
// Inputs:  block - the hardware timer block to use.
// Outputs:  none.
void Timer_ResetEdgeCountTimer(TimerBlock_t block);

//------------------ Timer_ReadCounterValue --------------------------
// Reads the accumulator of a timer configured for input edge counter mode.
// Inputs:  block - the hardware timer block to use.
// Outputs:  accumlator value.
uint32_t Timer_ReadEdgeCountValue(TimerBlock_t block);
#endif



#endif
