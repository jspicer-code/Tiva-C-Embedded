// File:  ControlSettings.h
// Author: JSpicer
// Date:  11/25/17
// Purpose: Control settings structure
// Hardware:  TM4C123 Tiva board

#ifndef CONTROL_SETTINGS_H
#define CONTROL_SETTINGS_H

#include "thermistor.h"

typedef struct {
	ThermScale_t scale;
	int calibrationOffset;
	int lowTemp;
	int highTemp;
} ControlSettings_t;


#endif
