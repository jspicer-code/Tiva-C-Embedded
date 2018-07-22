// File:  HAL_PLL.c
// Author: JSpicer
// Date:  9/9/17
// Purpose: PLL and bus clock utilities
// Hardware:  TM4C123, TM4C1294 Tiva board

#include <stdint.h>
#include "HAL.h"
#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else // TM4C123
#include "tm4c123gh6pm.h"
#endif

#define DEFAULT_FREQ		16000000

// Holds the currently configured bus clock frequency.
uint32_t PLL_BusClockFreq = DEFAULT_FREQ;

// Stores the number of ticks in 10ms, 10us, and 100ns based on the bus clock frequency.
uint32_t PLL_NumTicksPer10ms = DEFAULT_FREQ / 100;
uint32_t PLL_NumTicksPer10us = DEFAULT_FREQ / 100000;
uint32_t PLL_NumTicksPer100ns = DEFAULT_FREQ / 10000000;


static void SetBusClockFreq(uint32_t freq)
{
	PLL_BusClockFreq = freq;
	
	// Establish the number of ticks in 10ms for later use...
	PLL_NumTicksPer10ms = PLL_BusClockFreq / 100; //* 0.01;

	// Establish the number of ticks in 10us for later use...
	PLL_NumTicksPer10us = PLL_BusClockFreq / 100000; //* 0.00001;

	// Establish the number of ticks in 100ns for later use...
	PLL_NumTicksPer100ns = PLL_BusClockFreq / 10000000; //* 0.0000001;
	
}


#if (halCONFIG_1294 == 1)

//----------------------------------- PLL_Init ---------------------------------------
// Initializes the PLL to generate the requested SysClk frequency.  Assumes this will
// be called immediately after POR and not more than once.
//
// Inputs:  none.
// Outputs:  none.
// Notes:  The steps here were derived from information in the 1294 data sheet, 
//         in particular section 5.3 Initialization and Configuration, and also 
//         the source code for SysCtlClockFreqSet in sysctl.c and the PLL example from
//				 Valvano's examples at http://users.ece.utexas.edu/~valvano/arm/PLL_4C1294.zip.
//         There were differences in each source.  The implementation here attempts
//         fix an apparent typo in the data sheet, remove what seems to be unncessary steps,
//				 and reorder them where it's feasible and makes sense to do so.  

//         After power-on reset (POR) the CPU will be using the precision internal
//         oscillator (PIOSC) which runs at 16MHz.  There are three main steps to 
//         initializing/using the PLL:
//            1) Enable the main oscillator (MOSC).
//            2) Configure and enable the PLL.
//            3) Reconfigure SysClk to use the PLL as its source.
//         Each main step has one or more substeps involving direct register access. 
//
void PLL_Init(SysClkFreq_t freq)
{	
	//
	// STEP 1:  Enable the MOSC
	//
	
	// Apply sub-steps 1-3 together to preserve the current MOSCCTL bit values.
	uint32_t mosc = SYSCTL_MOSCCTL_R;
	
	// 1) Power up the MOSC (main oscillator) by clearing the "no crystal" bit.
	// The crystal on the EK-TM4C1294XL board is 25MHz and connected to OSC0 and OSC1.
	mosc &= ~SYSCTL_MOSCCTL_NOXTAL;
	
	// 2) Enable power to the main oscillator by clearing the power down bit.
	mosc &= ~SYSCTL_MOSCCTL_PWRDN;
	
	// 3) Specify high oscillator range (>= 10 MHz).
	// This substep is peformed by SysCtlClockFreqSet, which says is to "Increase
	// the drive strength for MOSC of 10MHz and above".  It is not used by Valvano
  // or mentioned in the data sheet configuration, but should in theory be set.
	mosc |= SYSCTL_MOSCCTL_OSCRNG;
	
	// 3) Set and wait until sufficient time has passed for the MOSC to reach the expected 25MHz frequency
	// of the crystal.  The resulting MOSCCTL value will be 0x10.
	SYSCTL_MOSCCTL_R = mosc;
	
	//This is checking the power up masked interrupt status bit.  
	while((SYSCTL_RIS_R & SYSCTL_RIS_MOSCPUPRIS) == 0) {}
	
	//
  // STEP 2:  Configure and enable the PLL
  //
		
	// 5) Clear and set the PLL input clock source to be the MOSC.
	// It appears that this substep is not mentioned in the data sheet.  It says
  // to set the OSCSRC field instead.  SysCtlClockFreqSet and Valvano both set PLLSRC, 
  // but then they also set OSCSRC as well. SysCtlClockFreqSet restores OSCSRC after
  // enabling the PLL due errata #23, but Valvano does not.  It seems unnecessary to ever
	// set OSCSRC given that the PLL will be used.  There is no explanation in the
	// datasheet as to why OSCCRC must be configured to use MOSC temporarily while the PLL
	// is being configured, so leaving it out here.
	SYSCTL_RSCLKCFG_R  = (SYSCTL_RSCLKCFG_R & ~SYSCTL_RSCLKCFG_PLLSRC_M) | SYSCTL_RSCLKCFG_PLLSRC_MOSC;
						
	// Steps 6-8 are described in the data sheet tables (pgs. 237-238)
	// fin = fxtal / ((Q+1)(N+1))	
	// MDIV = MINT + (MFRAC / 1024)
	// fvco = fin * MDIV
	// SysClk = fvco / (PSYSDIV + 1)
	
	// 6) For fxtal= 25MHz, set Q = 0, N = 4 => fin = 25MHz/((0+1)(4+1)) = 5MHz
	SYSCTL_PLLFREQ1_R = 0x4;

  // 7) Set MFRAC = 0		
	SYSCTL_PLLFREQ0_R &= ~SYSCTL_PLLFREQ0_MFRAC_M;
		
	// 8) Set MINT = 96 = 0x60 => fvco = (5MHz * 96) = 480MHz
	SYSCTL_PLLFREQ0_R = (SYSCTL_PLLFREQ0_R & ~SYSCTL_PLLFREQ0_MINT_M) | 0x60;
		
	// 9) Power up the PLL.  It will take some time to settle and lock the requested frequency.
	SYSCTL_PLLFREQ0_R |= SYSCTL_PLLFREQ0_PLLPWR;	

	// 10) Wait until the PLL is powered and locked.
	while ((SYSCTL_PLLSTAT_R & SYSCTL_PLLSTAT_LOCK) == 0) {}
	
  //
  // STEP 3:  Reconfigure SysClk to use the PLL.  
  //
		
  // 11) Set the timing parameters for the main Flash and EEPROM memories.
  // These settings will take effect once the MEMTIMU bit is set below.
	// Clear the relevant field bits making sure to leave the reserved bits unchanged.
	uint32_t memtim0 = SYSCTL_MEMTIM0_R;
	memtim0 &= ~(SYSCTL_MEMTIM0_EBCHT_M | SYSCTL_MEMTIM0_EBCE | SYSCTL_MEMTIM0_EWS_M | 
               SYSCTL_MEMTIM0_FBCHT_M | SYSCTL_MEMTIM0_FBCE | SYSCTL_MEMTIM0_FWS_M);
	
	uint32_t physdiv;
	uint32_t hz; 		

	switch (freq) {
		
		case SYSCLK_80:
			// For CPU frequency 60MHz < f <= 80MHz:
			//	EBCHT = FBCHT = 0x4
			//  EBCE = FBCE = 0
			//  FWS = EWS = 0x3;
			// Since the reserved bits 20 and 4 are showing 0x1, the resulting MEMTIM0 value should be:  0x01130113
			memtim0 |= (SYSCTL_MEMTIM0_EBCHT_2_5 | SYSCTL_MEMTIM0_FBCHT_2_5 | (0x3 << SYSCTL_MEMTIM0_EWS_S) |  0x3);
			physdiv = 0x5;
		  hz = 80000000;
			break;
		
		case SYSCLK_120:
		default:
			// For CPU frequency 100MHz < f <= 120MHz:
			//	EBCHT = FBCHT = 0x6
			//  EBCE = FBCE = 0
			//  FWS = EWS = 0x5;
			// Since the reserved bits 20 and 4 are showing 0x1, the resulting MEMTIM0 value should be:  0x01950195
			memtim0 |= (SYSCTL_MEMTIM0_EBCHT_3_5 | SYSCTL_MEMTIM0_FBCHT_3_5 | (0x5 << SYSCTL_MEMTIM0_EWS_S) |  0x5);
			physdiv = 0x3;
		  hz = 120000000;
			break;
	}
	
	SYSCTL_MEMTIM0_R = memtim0;
										 	
	// Apply steps 12-14 together.
	uint32_t rsclkcfg = SYSCTL_RSCLKCFG_R;
	
	// 12) Set the PLL System Clock divisor, e.g. 3 => SysClk = 480MHz / (1+3) = 120MHz.
	rsclkcfg = (rsclkcfg & ~SYSCTL_RSCLKCFG_PSYSDIV_M) | physdiv;		
		
	// 13) Use PLL as the system clock source
	rsclkcfg |= SYSCTL_RSCLKCFG_USEPLL;
		
	// 14) Apply the MEMTIMU register value and upate the memory timings set in MEMTIM0.
	rsclkcfg |= SYSCTL_RSCLKCFG_MEMTIMU;		
		
	SYSCTL_RSCLKCFG_R = rsclkcfg;
		
	// Save the bus frequency.
	SetBusClockFreq(hz);
	
}


#endif


#if (halCONFIG_1294 == 0)  // TM4C123
//------------------------- PLL_Init ----------------------------
// Initializes the PLL to a bus clock frequency.
// NOTE:  currently only supports 80Hz
// Inputs:  none.
// Outputs:  none.
void PLL_Init(SysClkFreq_t frequency)
{
	// 0) Set USERCC2 bit in order to use RCC2
	SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
	
	// 1) Set the BYPASS bit to bypass PLL while initializing.
	SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;

	// 2) Clear and set the XTAL bits to 16MHz
	SYSCTL_RCC_R = ((SYSCTL_RCC_R & ~SYSCTL_RCC_XTAL_M) | SYSCTL_RCC_XTAL_16MHZ);
	
	// 3)	Clear the OSCSRC2 bits to configure for Main OSC
	SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;
	
	// 4) Activate PLL by clearing PWRDN.
	SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
	
	// 5) Use 400 MHz PLL
	SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400 ;
	
	// 6) Clear the SYSDIV2 bits and set to 0x4 = 80MHz
	SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~(SYSCTL_RCC2_SYSDIV2_M | SYSCTL_RCC2_SYSDIV2LSB)) | (0x4 << 22);  
	
	// 7) Wait for the PLL to lock
	while ((SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) == 0);
	
	// 8) Clear BYPASS to enable PLL.
	SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
	
	// The bus frequency will be 80MHz.
	SetBusClockFreq(80000000);

}
#endif

