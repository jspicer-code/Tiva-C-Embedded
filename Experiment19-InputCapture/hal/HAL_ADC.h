// File:  HAL_ADC.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: ADC utilities
// Hardware:  TM4C123 Tiva board

#ifndef ADC_HAL_H
#define ADC_HAL_H

#include <stdint.h>

// These are names for the analog input channels.
typedef enum {
	AIN0 = 0,
	AIN1 = 1,
	AIN2 = 2,
	AIN3 = 3,
	AIN4 = 4,
	AIN5 = 5,
	AIN6 = 6,
	AIN7 = 7,
	AIN8 = 8,
	AIN9 = 9,
	AIN10 = 10,
	AIN11 = 11
} ADCChannel;

// These are names for the ADC modules.
typedef enum {
	ADC0 = 0,
	ADC1 = 1
} ADCModule;

// This structure is an abstract definition of an ADC module/channel combination.
typedef struct {
	ADCModule module;	
	ADCChannel channel;
} AnalogDef_t;

// Convenience macro for initializing an AnalogDef_t instance.
#define	ANALOGDEF(module, channel)		((AnalogDef_t){ module, channel })

//----------------------- ADC_Enable --------------------------
// Enables the specified ADC module/channel for input.  
//	This configures a single channel sequencer for single sample.
// Inputs:  module - the ADC module name.
//          channel - the ADC channel name.
// Outputs:  none.
void ADC_Enable(ADCModule module, ADCChannel channel);


//----------------------- ADC_Sample --------------------------
// Reads the digital output of the enabled ADC module.
// Inputs:  module - the ADC module name.
// Outputs:  none.
uint32_t ADC_Sample(ADCModule module);

#endif


