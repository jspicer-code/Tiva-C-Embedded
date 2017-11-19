// File:  Display.h
// Author: JS
// Date:  11/18/17
// Purpose: Multiplexed display service module.
// Hardware:  TM4C123 Tiva board

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void Display_Initialize(void);
void Display_Update(uint32_t value);
void Display_Blank(void);

#endif

