// File:  RTC.h
// Author: JSpicer
// Date:  8/11/18
// Purpose: Driver for the Maxim DS1307 real-time clock
// Hardware:  TM4C123 & TM4C1294 Tiva board
//
#ifndef RTC_H
#define RTC_H

#include "HAL.h"

// BCD values are converted to integer values and stored in separate fields.
// The CH, 12-hour mode, and AM/PM fields are stored in their own fields.
typedef struct {
	uint8_t		seconds;		// 0-59
	uint8_t		minutes;		// 0-59
	uint8_t		hours;			// 1-12
	uint8_t		day;				// 1=Sun, ..., 7=Sat
	uint8_t		date;				// 1-31
	uint8_t		month;			// 1-12
	uint8_t		year;				// 0-99, offset from 1970

	uint8_t		clockHalt;	// 1=Halted, 0=Running
	uint8_t 	hourMode; 	// 0=24hr, 1=12hr
	uint8_t		am_pm;			// 0=AM, 1=PM
} RTC_Clock_t;

bool RTC_Init(I2C_Module_t i2cModule, bool enableOutput, bool* isReset);
bool RTC_Read(RTC_Clock_t* clock);
bool RTC_Write(const RTC_Clock_t* clock);

#endif
