// File:  TemperatureSettings.h
// Author: JSpicer
// Date:  11/25/17
// Purpose: Temperature settings structure
// Hardware:  TM4C123 Tiva board

#ifndef TEMPERATURE_SETTINGS_H
#define TEMPERATURE_SETTINGS_H

#include "thermistor.h"

// Contains temperature mode settings.
typedef struct {
	
	// Holds the characters 'F', 'A', 'N', 'C" when written out to Flash.
	//	If these characters are not read from Flash then the rest of the
	//	structure is invalid.
	char magic[4];

	// Temperature scale (Fahrenheit or Celsius)
	ThermScale_t scale;
	
	// An integer value to added to the temperature reading.
	int calibrationOffset;
	
	// The lower temperature limit.
	int lowTemp;
	
	// The lower temperature limit.
	int highTemp;

} TemperatureSettings_t;


#endif
