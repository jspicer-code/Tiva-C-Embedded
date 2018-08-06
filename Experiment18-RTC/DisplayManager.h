#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include "LCD.h"

typedef struct {
	uint8_t		seconds;
	uint8_t		minutes;
	uint8_t		hours;
	uint8_t		am_pm;
	uint8_t		day;
	uint8_t		date;
	uint8_t		month;
	uint8_t		year;	
} Display_Clock_t;


int Display_Init(const LCDPinConfig_t* lcdConfig);
void Display_UpdateClock(const Display_Clock_t* clock);
void Display_GetClock(Display_Clock_t* clock);
void Display_EnableCursor(bool enable);
void Display_ResetCursorPosition(void);
void Display_NextField(void);
void Display_PrevField(void);
void Display_NextValue(void);
void Display_PrevValue(void);

#endif
