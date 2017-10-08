// File:  PWM_HAL.c
// Author: JS
// Date:  10/04/17
// Purpose: PWM utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include <assert.h>
#include "HAL.h"
#include "tm4c123gh6pm.h"

#define TIMER0_REG_BASE 	((volatile uint32_t *)0x40030000)
#define TIMER1_REG_BASE		((volatile uint32_t *)0x40031000)
#define TIMER2_REG_BASE		((volatile uint32_t *)0x40032000)
#define TIMER3_REG_BASE		((volatile uint32_t *)0x40033000)
#define TIMER4_REG_BASE		((volatile uint32_t *)0x40034000)
#define TIMER5_REG_BASE		((volatile uint32_t *)0x40035000)	


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
	uint32_t  TAPV;
	uint32_t  TBPV;
	uint32_t  RESERVED1[981];
	uint32_t  PP;
} TimerRegs_t;


const volatile uint32_t * TimerBaseAddress[] = {
	TIMER0_REG_BASE,
	TIMER1_REG_BASE,
	TIMER2_REG_BASE,
	TIMER3_REG_BASE,
	TIMER4_REG_BASE,
	TIMER5_REG_BASE,
};

PFN_TimerCallback Timer1_Callback;
PFN_TimerCallback Timer2_Callback;

int Timer_EnableTimerPeriodic(TimerBlock_t block, uint32_t interval, PFN_TimerCallback callback)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	switch (block) {
		
		case TIMER1:
			SYSCTL_RCGCTIMER_R |= 2;	
			break;
		
		case TIMER2:
			SYSCTL_RCGCTIMER_R |= 4;	
			break;
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			return -1;
	}
		
	// Disable while configuring.
	timer->CTL = 0;

	// 32-bit mode.
	timer->CFG = 0;

	// periodic down-counter.
	timer->TAMR = 0x02;

	// No scaler
	timer->TAPR = 0;

	// Load interval register
	timer->TAILR = interval;

	// Clear timeout flag
	timer->ICR = 0x1;

	// Enable timeout interrupt
	timer->IMR = 0x1;
	
	// Enable timer
	timer->CTL |= 0x01;
	
	switch (block) {
		
		case TIMER1:
			//
			// This is IRQ 21.  Note that the handler function (in this file) must be assigned to the
			// Vectors table in the startup_*.S file and specified in that file with an EXTERN directive
			// (so the linker can find it).
			// 
			// Register 34: Interrupt 20-23 Priority (PRI5), offset 0x414
			NVIC_PRI5_R = (NVIC_PRI5_R & 0xFFFF00FF) | 0x00006000; // priority 3
		
			Timer1_Callback = callback;

			// Enable...
			NVIC_EN0_R |= 0x00200000; // bit 21
		
			break;
		
		case TIMER2:
			//
			// This is IRQ 23.  Note that the handler function (in this file) must be assigned to the
			// Vectors table in the startup_*.S file and specified in that file with an EXTERN directive
			// (so the linker can find it).
			// 
			// Register 34: Interrupt 20-23 Priority (PRI5), offset 0x414
			NVIC_PRI5_R = (NVIC_PRI5_R & 0x00FFFFFF) | 0x60000000; // priority 3
		
			Timer2_Callback = callback;

			// Enable...
			NVIC_EN0_R |= 0x00800000; // bit 23
		
			break;
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			return -1;
	}
	
	return 0;
}	


int Timer_InitInputCounter(TimerBlock_t block)
{
	switch (block) {
		
		case TIMER1:
			SYSCTL_RCGCTIMER_R |= 2;	
			GPIO_InitPort(PORTB);
			GPIO_EnableAltDigital(PORTB, PIN_4, 0x7);
			break;
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			return -1;
	}
	
	return 0;
}	

void Timer_ResetInputCounter(TimerBlock_t block)
{
	
	// NOTE:  Assumes that the timer has been already been initialized!
	
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable while configuring.  This also sets the TAEVENT bits to 0 for rising edge trigger.
	timer->CTL = 0;

	// 16-bit mode.
	timer->CFG = 4;

	// up-count, edge-count, capture mode
	timer->TAMR = 0x13;

	// Set the count limit in the match and pre-scale match registers for a 24-bit limit.
	timer->TAMATCHR = 0xFFFF;
	timer->TAPMR = 0xFF;
	
	// Clear the value.
	timer->TAV = 0;
	timer->TAPV = 0;
	
	// Enable timer
	timer->CTL |= 0x01;
	
}

uint32_t Timer_ReadCounterValue(TimerBlock_t block)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	volatile uint32_t count = 0x00FFFFFF & ((timer->TAPR << 16) | timer->TAR);
	return count; 
}

static void TimerHandler(volatile TimerRegs_t* timer, PFN_TimerCallback callback)
{
	volatile int readback;

	if (timer->MIS & 0x1) {
		
		// Clear the timer timeout flag.
		timer->ICR = 0x1;
		
		// Read to force clearing of the interrupt flag.
		readback = timer->ICR;
		
		// Invoke the call back if registered.
		if (callback) {
			callback();
		}

	}
	else {
		
		// Shoudn't get here, but just in case...
		timer->ICR = timer->MIS;
		readback = timer->ICR;

	}
}


void Timer_Timer1Handler(void)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[TIMER1];
	TimerHandler(timer, Timer1_Callback);
}

void Timer_Timer2Handler(void)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[TIMER2];
	TimerHandler(timer, Timer2_Callback);
}
