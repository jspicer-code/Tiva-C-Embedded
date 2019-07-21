// File:  HAL_Timer.c
// Author: JSpicer
// Date:  10/04/17
// Purpose: Timer utilities
// Hardware:  TM4C123 & TM4C1294 Tiva board

#include <stdint.h>
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
	uint32_t  TAPV;
	uint32_t  TBPV;
	uint32_t  RESERVED1[981];
	uint32_t  PP;
#endif

} TimerRegs_t;

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

// This structure holds information for the interrupt handler.
typedef struct {
	TimerBlock_t block;
	TimerMode_t mode;
	PFN_TimerCallback callback;
	void* callbackData;
} HandlerInfo_t;

// Forward declaration, defined at the bottom of this file.
static void HandleInterrupt(HandlerInfo_t* info);

#define DEFINE_IRQ_HANDLER(name) \
static HandlerInfo_t name##_HandlerInfo; \
void Timer_##name##Handler(void) \
{ \
	HandleInterrupt(&name##_HandlerInfo); \
}

#if (halUSE_TIMER0 == 1)
#define IRQ_HANDLER_DEFINED
// TIMER0 interrupt handler.  
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER0)
#endif

#if (halUSE_TIMER1 == 1)
#define IRQ_HANDLER_DEFINED
// TIMER1 interrupt handler.
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER1)
#endif

#if (halUSE_TIMER2 == 1)
#define IRQ_HANDLER_DEFINED
// TIMER2 interrupt handler.  
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER2)
#endif

#if (halUSE_TIMER3 == 1)
#define IRQ_HANDLER_DEFINED
// TIMER3 interrupt handler.  
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER3)
#endif


#define ENABLE_IRQ(name, mode, irqConfig, useTimerA, useTimerB) \
{ \
	name##_HandlerInfo.block = name; \
	name##_HandlerInfo.mode = mode; \
	name##_HandlerInfo.callback = irqConfig->callback; \
	name##_HandlerInfo.callbackData = irqConfig->callbackData; \
	if (useTimerA) { NVIC_EnableIRQ(INT_##name##A - 16, irqConfig->priority); } \
	if (useTimerB) { NVIC_EnableIRQ(INT_##name##B - 16, irqConfig->priority); } \
}

static void EnableIRQHandler(TimerBlock_t block, TimerMode_t mode, const TimerIRQConfig_t* irqConfig, bool useTimerA, bool useTimerB)
{
	switch (block) {

#if (halUSE_TIMER0 == 1)
		case TIMER0:
			ENABLE_IRQ(TIMER0, mode, irqConfig, useTimerA, useTimerB);
			break;
#endif
		
#if (halUSE_TIMER1 == 1)
		case TIMER1:
			ENABLE_IRQ(TIMER1, mode, irqConfig, useTimerA, useTimerB);
			break;
#endif
	
#if (halUSE_TIMER2 == 1)		
		case TIMER2:
			ENABLE_IRQ(TIMER2, mode, irqConfig, useTimerA, useTimerB);
			break;
#endif

#if (halUSE_TIMER3 == 1)		
		case TIMER3:
			ENABLE_IRQ(TIMER3, mode, irqConfig, useTimerA, useTimerB);
			break;
#endif
		
		default:
			// TODO: Implement other timers as needed.
			abort();
	}
}

#if (halUSE_INTERVAL_TIMERS == 1)

static int ConfigureIntervalTimer(TimerBlock_t block, TimerMode_t mode, const TimerIRQConfig_t* irqConfig)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];

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
		EnableIRQHandler(block, mode, irqConfig, true, false);
		
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

#if (halUSE_EDGE_TIMERS == 1)

static int ConfigureEdgeTimer(TimerBlock_t block, const TimerIRQConfig_t* irqConfig, const PinDef_t* pinConfig)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
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
	
#if (halCONFIG_1294 == 1)
	uint8_t pinCtl = 0x3;
#else
	uint8_t pinCtl = 0x7;
#endif
	
	// Enable the associated GPIO pin (CCP) for capture event. 
	GPIO_EnableAltDigital(pinConfig->port, pinConfig->pin, pinCtl, false);

	// Clear interrupts for Timer A capture event and Timer B time-out.
	regs->ICR = (TIMER_ICR_CAECINT | TIMER_ICR_TBTOCINT);
	
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
		EnableIRQHandler(block, TIMER_EDGE_TIMER, irqConfig, true, true);
		
		// Enable interrupts for Timer A capture event and Timer B time-out.
		// The timeout event for Timer B occurs when its down counter reaches 0x0.
		regs->IMR = TIMER_IMR_CAEIM | TIMER_IMR_TBTOIM;
	}
	
	return 0;
}	

int Timer_StartEdgeTimer(TimerBlock_t block)
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

	return 0;
}	

#endif


// TODO:  The counter timer implementation is old and has not been tested since much refactoring of timers.
#if (halUSE_COUNTER_TIMERS == 1)

int Timer_EnableInputCounter(TimerBlock_t block, PinDef_t pinDef)
{
	// TODO:  The input counter functions have not been tested since refactored for the 1294.
	
	int retVal = 0;
	
#if (halCONFIG_1294 == 1)
	uint8_t ctl = 0x3;
#else	// TM4C123
	uint8_t ctl = 0x7;
#endif
	
	switch (block) {

#if (halUSE_TIMER1 == 1)
		case TIMER1:
			SYSCTL_RCGCTIMER_R |= 2;	
			break;
#endif
	
#if (halUSE_TIMER2 == 1)		
		case TIMER2:
			SYSCTL_RCGCTIMER_R |= 4;	
			break;
#endif
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			retVal = -1;
	}
	
	if (retVal == 0) {
		GPIO_EnableAltDigital(pinDef.port, pinDef.pin, ctl, false);
	}
	
	return retVal;
}	

void Timer_ResetInputCounter(TimerBlock_t block)
{
	// NOTE:  Assumes that the timer has been already been initialized!
	// TODO:  The input counter functions have not been tested since refactored for the 1294.
	
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable while configuring.  This also sets the TAEVENT bits to 0 for rising edge trigger.
	regs->CTL = 0;

	// 16-bit mode.
	regs->CFG = 4;

	// up-count, edge-count, capture mode
	regs->TAMR = 0x13;

	// Set the count limit in the match and pre-scale match registers for a 24-bit limit.
	regs->TAMATCHR = 0xFFFF;
	regs->TAPMR = 0xFF;
	
	// Clear the value.
	regs->TAV = 0;
	
#if (halCONFIG_1294 != 1)	// TM4C123
	regs->TAPV = 0;
#endif 

	// Enable timer
	regs->CTL |= 0x01;
	
}

uint32_t Timer_ReadCounterValue(TimerBlock_t block)
{
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	volatile uint32_t count = 0x00FFFFFF & ((regs->TAPR << 16) | regs->TAR);
	return count; 
}

#endif

int Timer_Init(TimerBlock_t block, TimerMode_t mode, const TimerIRQConfig_t* irqConfig, const PinDef_t* pinConfig)
{
	
	// Enable the clock to the timer block.
	uint32_t clockBit = (uint32_t)(0x1 << (uint32_t)block);
	SYSCTL_RCGCTIMER_R |= clockBit;	
	
	// Wait for the peripheral ready bit to be set.  If the code doesn't wait
	//	it will hang up when it attempts to set the CFG register below.
	while (!(SYSCTL_PRTIMER_R & clockBit)) {};
	
	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable the timer block while configuring.  Specifically, clear the TAEN and TBEN (enable) bits.
	regs->CTL = 0;

	// Clear and disable interrupts
	regs->IMR = 0;
	
	switch (mode) {
				
#if (halUSE_INTERVAL_TIMERS == 1)
		case TIMER_ONESHOT:
		case TIMER_PERIODIC:
			ConfigureIntervalTimer(block, mode, irqConfig);
			break;
#endif		
		
#if (halUSE_EDGE_TIMERS == 1)
		case TIMER_EDGE_TIMER:
			ConfigureEdgeTimer(block, irqConfig, pinConfig);
			break;
#endif
		
#if (halUSE_COUNTER_TIMERS == 1)
		case TIMER_COUNTER:
			break;
#endif		

		default:
			break;
	}
			
	// Disable interrupts if no configuration.
	if (!irqConfig) {
		regs->IMR = 0;
	}

	return 0;
}	

void Timer_Stop(TimerBlock_t block)
{	
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];	
	timer->CTL = 0;
}


#ifdef IRQ_HANDLER_DEFINED
// Generic handler (called by all timer interrupt handlers)
static void HandleInterrupt(HandlerInfo_t* info)
{
 	volatile TimerRegs_t* regs = (volatile TimerRegs_t*)TimerBaseAddress[info->block]; 
	volatile int readback;
	
	TimerEventArgs_t args = { info->block, info->callbackData, TIMER_NO_EVENT, 0 };

	// From the 1294 user guide:
	// 	In capture down count modes, bits 15:0 contain the value of the counter and bits 23:16 contain
	// 	the current, free-running value of the prescaler, which is the upper 8 bits of the count.
	// 	In one-shot or periodic down count modes, the prescaler stored in 23:16 is a true prescaler, 
	// 	meaning bits 23:16 count down before decrementing the value in bits 15:0.

	// There are two events that can occur:  timeout events and edge events.  
	// Check first to see if a timeout event has occurred. For edge timers, this will be Timer B.
	// For regular 32-bit (concatenated) interval timers, this will be Timer A .
	uint32_t timeoutMask = (info->mode == TIMER_EDGE_TIMER) ? TIMER_MIS_TBTOMIS : TIMER_MIS_TATOMIS;
	
	if (regs->MIS & timeoutMask) {
		
		// Clear the timer timeout flag.
		regs->ICR = timeoutMask;
		
		args.eventType = TIMER_TIMEOUT_EVENT;
	}
	else if (regs->MIS & TIMER_MIS_CAEMIS) {
		
		// Clear the timer capture flag.
		regs->ICR = TIMER_MIS_CAEMIS;
		
		// Get the captured edge time.  In the 16-bit Input Edge Time mode, bits 15:0 contain the value
		// of the counter and bits 23:16 contain the value of the prescaler, which is the upper 8 bits
		// of the count.  In other words, it isn't necessary to read and shift the TBPS (prescaler) register.
		args.eventData.timerCount = 0xFFFFFF & regs->TAR;
		args.eventType = TIMER_EDGE_EVENT;
		
	}
	else {
		// Shoudn't get here, but just in case...
		regs->ICR = regs->MIS;
	}
	
	// Read to force clearing of the interrupt flag.
	readback = regs->ICR;
		
	// Invoke the call back if registered.
	if (info->callback && args.eventType != TIMER_NO_EVENT) {
			info->callback(info->block, &args);
	}	
	
}
#endif
