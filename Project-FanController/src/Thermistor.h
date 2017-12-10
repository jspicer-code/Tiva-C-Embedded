// File:  Thermistor.h
// Author: JSpicer
// Date:  10/20/17
// Purpose: Thermistor utilities for the Vishay NTCASCWE3 thermistor.
// Hardware:  TM4C123 Tiva board and Vishay NTCASCWE3 thermistor.

#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <stdint.h>

// Temperature scale names.
typedef enum {
	THERM_FAHRENHEIT,
	THERM_CELSIUS
} ThermScale_t;

//----------------------- Therm_GetTemperature --------------------------
// Gets the temperature in the given scale for the Vishay NTCASCWE3
//	 thermistor based on an analog input. 
// Inputs:  adcSample - the digital output of an ADC, between 0-4095.
//          scale - the temperature scale, i.e. Fahrenheit or Celsius.
// Outputs:  the temperature in the given scale.
int Therm_GetTemperature(uint32_t adcSample, ThermScale_t scale);

#endif
