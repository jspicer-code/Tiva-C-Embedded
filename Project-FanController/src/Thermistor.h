// File:  Thermistor.h
// Author: JSpicer
// Date:  10/20/17
// Purpose: Thermistor utilities
// Hardware:  TM4C123 Tiva board

#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <stdint.h>

typedef enum {
	THERM_FAHRENHEIT,
	THERM_CELSIUS
} ThermScale_t;

// Get the temperature in the give scale based on the ADC output.
int Therm_GetTemperature(uint32_t adcSample, ThermScale_t scale);

#endif
