// File:  HAL_Timer.c
// Author: JSpicer
// Date:  10/04/17
// Purpose: Timer utilities
// Hardware:  TM4C123 & TM4C1294 Tiva board

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "HAL.h"

#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else  // TM4C123
#include "tm4c123gh6pm.h"
#endif

#if (halCONFIG_1294 == 1)
#define TIMER_MAX_BLOCKS	8
#else  // TM4C123
#define TIMER_MAX_BLOCKS	6
#endif

// These are the base IO addresses of the timer blocks.
#define TIMER0_REG_BASE 	((volatile uint32_t *)0x40030000)
#define TIMER1_REG_BASE		((volatile uint32_t *)0x40031000)
#define TIMER2_REG_BASE		((volatile uint32_t *)0x40032000)
#define TIMER3_REG_BASE		((volatile uint32_t *)0x40033000)
#define TIMER4_REG_BASE		((volatile uint32_t *)0x40034000)
#define TIMER5_REG_BASE		((volatile uint32_t *)0x40035000)	
#define TIMER6_REG_BASE		((volatile uint32_t *)0x400E0000)
#define TIMER7_REG_BASE		((volatile uint32_t *)0x400E1000)


// This array is a look up table to resolve the timer block name to its base address.
const volatile uint32_t * TimerBaseAddress[TIMER_MAX_BLOCKS] = {
	TIMER0_REG_BASE,
	TIMER1_REG_BASE,
	TIMER2_REG_BASE,
	TIMER3_REG_BASE,
	TIMER4_REG_BASE,
	TIMER5_REG_BASE,
#if (halCONFIG_1294 == 1)
	TIMER6_REG_BASE,
	TIMER7_REG_BASE
#endif
};

static void EnableClockSource(TimerBlock_t block)
{
	// Enable the clock to the timer block.
	uint32_t clockBit = (uint32_t)(0x1 << (uint32_t)block);
	SYSCTL_RCGCTIMER_R |= clockBit;	
	
	// Wait for the peripheral ready bit to be set.  If the code doesn't wait
	// it will hang up when it attempts to set the CFG register.
	while (!(SYSCTL_PRTIMER_R & clockBit)) {};
}		
		
static void EnableIRQHandler(TimerBlock_t block, TimerSubTimer_t subTimer, const TimerIRQConfig_t* irqConfig)
{
	
	struct { 
		uint8_t subTimer[2]; 
	} intNumbers[TIMER_MAX_BLOCKS] = {
		{ INT_TIMER0A, INT_TIMER0B },
		{ INT_TIMER1A, INT_TIMER1B },
		{ INT_TIMER2A, INT_TIMER2B },
		{ INT_TIMER3A, INT_TIMER3B },
		{ INT_TIMER4A, INT_TIMER4B },
		{ INT_TIMER5A, INT_TIMER5B },
#if (halCONFIG_1294 == 1)
		{ INT_TIMER6A, INT_TIMER6B },
		{ INT_TIMER7A, INT_TIMER7B },
#endif		
	};

	TimerISRData_t* data = (TimerISRData_t*)malloc(sizeof(TimerISRData_t));
	data->timerRegs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	data->timerState = irqConfig->callbackData;

	if (subTimer == TIMERA) {
		NVIC_EnableIRQ(intNumbers[block].subTimer[0] - 16, irqConfig->priority, data);
	}
	else {
		NVIC_EnableIRQ(intNumbers[block].subTimer[1] - 16, irqConfig->priority, data);
	}
	
}

#if (halUSE_INTERVAL_TIMERS == 1)

int Timer_InitIntervalTimer(TimerBlock_t block, TimerIntervalMode_t mode, const PinDef_t* pinConfig, const TimerIRQConfig_t* irqConfig)
{
	EnableClockSource(block);
	
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable the timer block while configuring.  Specifically, clear the TAEN and TBEN (enable) bits.
	regs->CTL = 0;

	// Disable and clear pending interrupts.
	regs->IMR = 0;
	regs->ICR = (TIMER_ICR_CAECINT | TIMER_ICR_TBTOCINT);
	
	// 32-bit mode.
	regs->CFG = 0;

	// D1:D0 bits: 0x02 periodic, 0x01 one shot.
	regs->TAMR = (mode == TIMER_PERIODIC) ? TIMER_TAMR_TAMR_PERIOD : TIMER_TAMR_TAMR_1_SHOT;

	// No pre-scaler
	regs->TAPR = 0;

	// Clear timeout flag
	regs->ICR = TIMER_ICR_TATOCINT;
	
	if (irqConfig) {
	
		// Enable IRQ handler for timer A only.
		EnableIRQHandler(block, TIMERA, irqConfig);
		
		// Unmask timeout interrupt.
		regs->IMR = TIMER_IMR_TATOIM;
	}
	
	return 0;
}

void Timer_Wait(TimerBlock_t block, uint32_t count, uint32_t interval)
{	
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	for (uint32_t i = 0; i < count; i++) {
	
		Timer_StartInterval(block, interval);

		// Wait for the time-out raw interrupt bit to be set.
		while ((regs->RIS & 0x1) == 0);

		// Clear the raw interrupt flag.
		regs->ICR = 0x1;
		
	}
}

int Timer_StartInterval(TimerBlock_t block, uint32_t interval)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable while configuring.
	regs->CTL = 0;

	// Interval load register
	regs->TAILR = interval - 1;

	// Clear timeout flag
	regs->ICR = TIMER_ICR_TATOCINT;
	
	// Enable timer
	regs->CTL |= TIMER_CTL_TAEN;
		
	return 0;
}	

#endif

#if (halUSE_EDGE_TIME_TIMERS == 1)

int Timer_InitEdgeTimeTimer(TimerBlock_t block, TimerEventType_t eventType, const PinDef_t* pinConfig, const TimerIRQConfig_t* irqConfig)
{
	EnableClockSource(block);

	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
		// Disable the timer block while configuring.  Specifically, clear the TAEN and TBEN (enable) bits.
	regs->CTL = 0;

	// Disable and clear pending interrupts.
	regs->IMR = 0;
	regs->ICR = (TIMER_ICR_CAECINT | TIMER_ICR_TBTOCINT);
	
	// Selects the 16-bit timer configuration.  This allows sub-timers A and B to be used simultaneously.
	regs->CFG = TIMER_CFG_16_BIT;

	// Set timer A mode field (TAMR) for capture (CAP) and capture mode bit (TACMR) for edge time.  
	// This also clears the count direction field (TACDIR) for count down.
	regs->TAMR = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP; 

	// Set timer B mode field (TBMR) for periodic mode.  This also clears the count direction field (TBCDIR)
	// for count down.
	regs->TBMR = TIMER_TBMR_TBMR_PERIOD;
		
	// For timers A and B, set the pre-scale (8-bit) and interval load registers (16-bit) to maximum values,
	// making them each effectively 24-bit count-down timers.
	regs->TAPR = 0xFF;
	regs->TAILR = 0xFFFF;
	regs->TBPR = 0xFF;
	regs->TBILR = 0xFFFF;
	
	// Clear the event field so that it defaults to postive edge, then set accordingly.
	regs->CTL &= ~(TIMER_CTL_TAEVENT_M);
	if (eventType == TIMER_EVENT_BOTH_EDGES) {
		regs->CTL |= TIMER_CTL_TAEVENT_BOTH;
	}
	else if (eventType == TIMER_EVENT_FALLING_EDGE) {
		regs->CTL |= TIMER_CTL_TAEVENT_NEG;		
	}
	
#if (halCONFIG_1294 == 1)
	uint8_t pinCtl = 0x3;
#else
	uint8_t pinCtl = 0x7;
#endif
	
	// Enable the associated GPIO pin (CCP) for capture event. 
	GPIO_EnableAltDigital(pinConfig->port, pinConfig->pin, pinCtl, false);
	
	if (irqConfig) {
	
		// Enable the IRQ handlers for timers A and B.
		// From the TMC1294 User Guide concerning the Exception Model of the Cortex M4 (1294 pg. 120):
		// 	1) If an exception occurs with the same priority as the exception being handled, the handler is not preempted.
		//	2) If multiple pending exceptions have the same priority, the pending exception with the lowest exception
		//     number takes precedence, irrespective of the exception number.
		// Timer A and Timer B will be given the same priority, but since Timer A has the lower IRQ #, it means that
		// when an edge capture interrupt (Timer A) and timeout interrupt (Timer B) occur at exactly the same time 
		// (i.e. TAV = TBV = 0), the interrupt handler will execute to completion first for Timer A, then for by Timer B.
		// If Timer B is handled first while Timer A is pending, then logically it means that the timeout event
		// occurred slightly before the edge event (e.g. an edge occured while inside Timer B's handler) and will
		// execute to completion before the edge capture is handled.
		EnableIRQHandler(block, TIMERA, irqConfig);
		EnableIRQHandler(block, TIMERB, irqConfig);
		
		// Enable interrupts for Timer A capture event and Timer B time-out.
		// The timeout event for Timer B occurs when its down counter reaches 0x0.
		regs->IMR = TIMER_IMR_CAEIM | TIMER_IMR_TBTOIM;
	}
	
	return 0;
	
}

void Timer_EnableEdgeTimeTimer(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Clear interrupts for Timer A capture event and Timer B time-out.
	regs->ICR = (TIMER_ICR_CAECINT | TIMER_ICR_TBTOCINT);

	// Note about synchronization:
	// When the timers are first enabled together, they are synchronized by default, so this step will be unnecessary.
	// If they are forced to drift apart (e.g. by disabling one timer, spin-looping, then disabling the other timer), 
	// they do not resynchronize automatically when they are renabled together.  In this case, setting the SYNC register
	// is required.  Syncing seems to work while both timers are disabled or enabled (and running).  To observe synchronization,
	// enable the STALL bits in the CTL register, then the set a breakpoint in the ISR.  The TAV and TBV registers will
	// be the same (except for the prescale value location in bits 23:16 for Timer A). See the User Guide notes for GPTMTAV/GPTMTBV. 
	// regs->CTL |= (TIMER_CTL_TASTALL | TIMER_CTL_TBSTALL);
	regs->SYNC = TIMER_SYNC_SYNCT0_M << (2 * ((int)block));

	// Enable timers A and B.  If they were already enabled, then they should now be resynchronized.
	regs->CTL |= (TIMER_CTL_TAEN | TIMER_CTL_TBEN);

}	

void Timer_DisableEdgeTimeTimer(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Clear interrupts for Timer A capture event and Timer B time-out.
	regs->ICR = (TIMER_ICR_CAECINT | TIMER_ICR_TBTOCINT);

	// Disable timers A and B. 
	regs->CTL &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);
}	


#endif


// TODO:  The counter timer implementation is old and has not been tested since much refactoring of timers.
#if (halUSE_EDGE_COUNT_TIMERS == 1)

static int ConfigureEdgeCountTimer(TimerBlock_t block, const TimerIRQConfig_t* irqConfig, const PinDef_t* pinConfig)
{
	int retVal = 0;
	
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Selects the 16-bit timer configuration.
	regs->CFG = TIMER_CFG_16_BIT;

	// Set timer A mode field (TAMR) for up-count, edge-count (TACMR is implicitly cleared), and capture mode.
	regs->TAMR = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; 
	
	// Set the count limit in the match and pre-scale match registers for a 24-bit limit.
	// Per the user guide, the total number of edge events counted is equal to the value
	// in TAPR/TAILR minus TAPMR/TAMATCHR.  So, for a full 24-bit up count TAPR/TAILR
	// should apparently be zeroed out.
	regs->TAPMR = 0xFF;
	regs->TAMATCHR = 0xFFFF;
	regs->TAPR = 0;
	regs->TAILR = 0;
	
	#if (halCONFIG_1294 == 1)
	uint8_t pinCtl = 0x3;
#else
	uint8_t pinCtl = 0x7;
#endif
	
	// Enable the associated GPIO pin (CCP) for capture event. 
	GPIO_EnableAltDigital(pinConfig->port, pinConfig->pin, pinCtl, false);
	
	// TODO:  Match interrupts have not been tested!
/*	
	// Clear match interrupt for Timer A
	regs->ICR = TIMER_ICR_CAMCINT;
	
	if (irqConfig) {
	
		// Enable the IRQ handler for timer A.
		EnableIRQHandler(block, TIMER_EDGE_COUNT, irqConfig, true, false);
		
		// Enable match interrupt for Timer A.
		regs->IMR = TIMER_IMR_CAMIM;
	}
*/	

	return retVal;
}

int Timer_EnableEdgeCountTimer(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Clear match interrupt for Timer A.
	regs->ICR = TIMER_ICR_CAMCINT;

	// Enable timers A.
	regs->CTL |= TIMER_CTL_TAEN;
	
	return 0;
}	

void Timer_DisableEdgeCountTimer(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Clear match interrupt for Timer A.
	regs->ICR = TIMER_ICR_CAMCINT;

	// Disable timers A and B. 
	regs->CTL &= ~TIMER_CTL_TAEN;
}	

void Timer_ResetEdgeCountTimer(TimerBlock_t block)
{
	// NOTE:  Assumes that the timer has been already been initialized!
	// TODO:  The input counter functions have not been tested since refactored for the 1294.
	
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable while resetting.
	regs->CTL = 0;

	// Clears the free-running counter.
	regs->TAV = 0;
	
	// Enable timer
	regs->CTL |= 0x01;
	
}

uint32_t Timer_ReadEdgeCountValue(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	uint32_t count = 0xFFFFFF & regs->TAR;
	return count; 
}

#endif


volatile TimerRegs_t* Timer_GetRegisters(TimerBlock_t block)
{
	return (volatile TimerRegs_t*)TimerBaseAddress[block];
}

bool Timer_IsTimerEnabled(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	return regs->CTL & TIMER_CTL_TAEN;
}	

// TODO: Find out where this function is/was called and change it so it clears only the EN bit.
//void Timer_Stop(TimerBlock_t block)
//{	
//	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];	
//	timer->CTL = 0;
//}

	
#if 0
void Timer_CaptureHandler()
{
	HandlerInfo_t* isrData = (HandlerInfo_t*)NVIC_GetActiveISRData();
		
 	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[isrData->block]; 
	
	// Clear the timer capture flag.
	regs->ICR = TIMER_MIS_CAEMIS;	
	
	// Read to force clearing of the interrupt cancel flag.
	volatile uint32_t readback = regs->ICR;

	// Invoke the callback...
	if (isrData->callback) {
		TimerEventArgs_t args = { isrData->block, isrData->callbackData, TIMER_EDGE_EVENT, regs };
		isrData->callback(&args);
	}	
}
#endif

#if 0
// Generic handler (called by all timer interrupt handlers)
void Timer_TimeoutHandler()
{
	HandlerInfo_t* isrData = (HandlerInfo_t*)NVIC_GetActiveISRData();
		
 	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[isrData->block]; 
	
	// Clear the timer timeout flag. For edge timers, this will be Timer B. For regular 32-bit (concatenated)
	// interval timers, this will be Timer A .
	regs->ICR = (isrData->mode == TIMER_EDGE_TIME) ? TIMER_MIS_TBTOMIS : TIMER_MIS_TATOMIS;
		
	// Read to force clearing of the interrupt cancel flag.
	volatile uint32_t readback = regs->ICR;
		
	// Invoke the callback...
	if (isrData->callback) {
		TimerEventArgs_t args = { isrData->block, isrData->callbackData, TIMER_TIMEOUT_EVENT, regs };
		isrData->callback(&args);
	}	

}
#endif
