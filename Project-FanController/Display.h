// File:  Display.h
// Author: JS
// Date:  10/28/17
// Purpose: Multiplexed display service module.
// Hardware:  TM4C123 Tiva board

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// This structure holds the io port bit-band addresses used by the display.
typedef struct {
	
	volatile uint32_t* bcdA;
	volatile uint32_t* bcdB;
	volatile uint32_t* bcdC;
	volatile uint32_t* bcdD;
	
	volatile uint32_t* digit0Enable;
	volatile uint32_t* digit1Enable;
	volatile uint32_t* digit2Enable;
	volatile uint32_t* digit3Enable;
	
} Display_IOPorts_t;

void Display_Initialize(const Display_IOPorts_t* io);
void Display_Update(uint32_t value);

#endif

