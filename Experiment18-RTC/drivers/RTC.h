
#ifndef RTC_H
#define RTC_H

#include "HAL.h"

typedef struct {
	uint8_t		seconds;
	uint8_t		minutes;
	uint8_t		hours;
	uint8_t		am_pm;
	uint8_t		day;
	uint8_t		date;
	uint8_t		month;
	uint8_t		year;	
} RTC_Clock_t;

void RTC_Init(I2C_Module_t i2cModule);
void RTC_Read(RTC_Clock_t* clock);
void RTC_Write(const RTC_Clock_t* clock);

#endif
