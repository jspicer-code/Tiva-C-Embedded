// File:  ADC_HAL.c
// Author: JS
// Date:  9/30/17
// Purpose: ADC utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include <assert.h>
#include "HAL.h"
#include "tm4c123gh6pm.h"

#define ADC0_REG_BASE 	((volatile uint32_t *)0x40038000)
#define ADC1_REG_BASE		((volatile uint32_t *)0x40039000)

typedef struct {
   uint32_t  ACTSS;
   uint32_t  RIS;
   uint32_t  IM;
   uint32_t  ISC;
   uint32_t  OSTAT;
   uint32_t  EMUX;
   uint32_t  USTAT;
   uint32_t  TSSEL;
   uint32_t  SSPRI;
   uint32_t  SPC;
   uint32_t  PSSI;
   uint32_t  RESERVED;
   uint32_t  SAC;
   uint32_t  DCISC;
   uint32_t  CTL;
   uint32_t  RESERVED1;
   uint32_t  SSMUX0;
   uint32_t  SSCTL0;
   uint32_t  SSFIFO0;
   uint32_t  SSFSTAT0;
   uint32_t  SSOP0;
   uint32_t  SSDC0;
   uint32_t  RESERVED2[2];
   uint32_t  SSMUX1;
   uint32_t  SSCTL1;
   uint32_t  SSFIFO1;
   uint32_t  SSFSTAT1;
   uint32_t  SSOP1;
   uint32_t  SSDC1;
   uint32_t  RESERVED3[2];
   uint32_t  SSMUX2;
   uint32_t  SSCTL2;
   uint32_t  SSFIFO2;
   uint32_t  SSFSTAT2;
   uint32_t  SSOP2;
   uint32_t  SSDC2;
   uint32_t  RESERVED4[2];
   uint32_t  SSMUX3;
   uint32_t  SSCTL3;
   uint32_t  SSFIFO3;
   uint32_t  SSFSTAT3;
   uint32_t  SSOP3;
   uint32_t  SSDC3;
   uint32_t  RESERVED5[786];
   uint32_t  DCRIC;
   uint32_t  RESERVED6[63];
   uint32_t  DCCTL0;
   uint32_t  DCCTL1;
   uint32_t  DCCTL2;
   uint32_t  DCCTL3;
   uint32_t  DCCTL4;
   uint32_t  DCCTL5;
   uint32_t  DCCTL6;
   uint32_t  DCCTL7;
   uint32_t  RESERVED7[8];
   uint32_t  DCCMP0;
   uint32_t  DCCMP1;
   uint32_t  DCCMP2;
   uint32_t  DCCMP3;
   uint32_t  DCCMP4;
   uint32_t  DCCMP5;
   uint32_t  DCCMP6;
   uint32_t  DCCMP7;
   uint32_t  RESERVED8[88];
   uint32_t  PP;
   uint32_t  PC;
   uint32_t  CC;
} ADCRegs_t;

const volatile uint32_t * ADCBaseAddress[] = {
	ADC0_REG_BASE,
	ADC1_REG_BASE,
};

void ADC_Enable(ADCModule module, ADCChannel channel)
{
		uint8_t moduleBit = 0x1 << ((uint8_t)module);
	
		switch (channel) {
			
			case AIN0:
				// Initialize PORTE and enable PE3
				GPIO_InitPort(PORTE);
				GPIO_EnableAltAnalog(PORTE, 0x08);
				break;
		
			case AIN1:
				// Initialize PORTE and enable PE2
				GPIO_InitPort(PORTE);
				GPIO_EnableAltAnalog(PORTE, 0x04);
				break;
			
			default:
				// TODO:  Implement for other channels as needed.
				abort();
				break;
		}
		
		// Enable the ADC module.
		SYSCTL_RCGCADC_R |= moduleBit;
		
		// Wait for the ADC peripheral to be ready...
		while (!(SYSCTL_PRADC_R & moduleBit )){}
		
		// Overlay the ADC register structure on top of the ADCx memory region...
		volatile ADCRegs_t* adc = (volatile ADCRegs_t*)ADCBaseAddress[module];
		
		// Disable SS3 during configuration
		adc->ACTSS &= ~0x08;
		
		// Software trigger conversion.
		adc->EMUX &= ~0xF000;
		
		// Select channel  
		adc->SSMUX3 = channel;
		
		// Single-ended, one conversion, raw interrupt.
		adc->SSCTL3 |= 6;
		
		// Enable SS3
		adc->ACTSS |= 0x08;
		
}



uint32_t ADC_Sample(ADCModule module)
{
	volatile uint32_t result;
	volatile ADCRegs_t* adc = (volatile ADCRegs_t*)ADCBaseAddress[module];
	
	// Begin sampling on SS3.
	adc->PSSI |= 0x08;
	
	// Wait for SS3 raw interrupt status to complete.
	while ((adc->RIS & 0x08) == 0);
	
	// Read conversion result.
	result = adc->SSFIFO3;
	
	// Clear completion flag.
	adc->ISC |= 0x08;
	
	return result;
}
