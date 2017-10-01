// File:  SevenSegDisplay.c
// Author: JS
// Date:  9/30/17
// Purpose: Segment and pattern definitions for a seven-segment display.
// Hardware:  TM4C123 Tiva board

#include <stdint.h>

const uint8_t SEG_A		= 0x80;
const uint8_t SEG_B		= 0x01;
const uint8_t SEG_C		= 0x04;
const uint8_t SEG_D		= 0x10;
const uint8_t SEG_E		= 0x20;
const uint8_t SEG_F		= 0x40;
const uint8_t SEG_G		= 0x02;
const uint8_t SEG_DP	= 0x08;
	
const uint8_t PATTERN_0 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const uint8_t PATTERN_1 = SEG_B | SEG_C;
const uint8_t PATTERN_2 = SEG_A | SEG_B | SEG_G | SEG_E | SEG_D;
const uint8_t PATTERN_3 = SEG_A | SEG_B | SEG_G | SEG_C | SEG_D;
const uint8_t PATTERN_4 = SEG_B | SEG_C | SEG_F | SEG_G;
const uint8_t PATTERN_5 = SEG_A | SEG_F | SEG_G | SEG_C | SEG_D;
const uint8_t PATTERN_6 = SEG_A | SEG_F | SEG_G | SEG_C | SEG_D | SEG_E;
const uint8_t PATTERN_7 = SEG_A | SEG_B | SEG_C;
const uint8_t PATTERN_8 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t PATTERN_9 = SEG_A | SEG_B | SEG_C | SEG_F | SEG_G;

const uint8_t DIGITS[10] = {
	PATTERN_0,
	PATTERN_1,
	PATTERN_2,
	PATTERN_3,
	PATTERN_4,
	PATTERN_5,
	PATTERN_6,
	PATTERN_7,
	PATTERN_8,
	PATTERN_9
};


