#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include "LCD.h"
#include "RTC.h"


int Display_Init(const LCDPinConfig_t* lcdConfig);
void Display_UpdateClock(const RTC_Clock_t* clock);
void Display_GetClock(RTC_Clock_t* clock);
void Display_EnableCursor(bool enable);
void Display_NextField(void);
void Display_PrevField(void);
void Display_NextValue(void);
void Display_PrevValue(void);

#endif
