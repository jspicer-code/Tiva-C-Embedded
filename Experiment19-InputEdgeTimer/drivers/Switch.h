// File:  Switch.h
// Author: JSpicer
// Date:  6/22/19
// Purpose:  Driver for polling and checking the status of switch (e.g. onboard button).
// Hardware:  TM4C123 & TM4C1294 Tiva board

#ifndef SWITCH_H
#define SWITCH_H

#include "HAL.h"

#define Switch_IsClosed(sw) ((sw).urrentState)
#define Switch_HasClosed(sw) ((sw).currentState && !(sw).previousState)
#define Switch_HasOpened(sw) (!(sw).currentState && (sw).previousState)

typedef enum {	
	SWITCH_CLOSED_LEVEL_HIGH,
	SWITCH_CLOSED_LEVEL_LOW
} Switch_ClosedLevel_t;	
	
typedef struct {	
	volatile uint32_t* address;
	Switch_ClosedLevel_t closedLevel;
	bool currentState;
	bool previousState;
} Switch_t;

void Switch_Init(Switch_t* sw, const PinDef_t* pinDef, Switch_ClosedLevel_t closedLevel);
void Switch_Poll(Switch_t* sw);

#endif
