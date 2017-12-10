// File:  Display.h
// Author: JSpicer
// Date:  11/18/17
// Purpose: Multiplexed display service module.
// Hardware:  TM4C123 Tiva board

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "HAL.h"

//----------------------- Display_Initialize --------------------------
// Initializes the display module.
// Inputs:  ssiModule - the SSI module to use in the HAL for SPI 
//            communication with the display shift register.
//          timerBlock - the HAL timer block to use for the refresh cycle.
// Outputs:  none
void Display_Initialize(SSIModule_t ssiModule, TimerBlock_t timerBlock);


//----------------------- Display_Update --------------------------
// Assigns a new value to be shown on the display.
// Inputs:  value - an integer value (0-9999) to show on the displayl.
// Outputs:  none.
void Display_Update(uint32_t value);


//----------------------- Display_Blank --------------------------
// Blanks the entire display (all digits off).
// Inputs:  none
// Outputs:  none
void Display_Blank(void);

#endif

