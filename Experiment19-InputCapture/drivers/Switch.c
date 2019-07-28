// File:  Switch.c
// Author: JSpicer
// Date:  6/22/19
// Purpose:  Driver for polling and checking the state of switch (e.g. onboard button).
// Hardware:  TM4C123 & TM4C1294 Tiva board

#include "Switch.h"

void Switch_Poll(Switch_t* sw)
{
	sw->previousState = sw->currentState;
	uint8_t level = *sw->address;
	sw->currentState = ((!level && sw->closedLevel == SWITCH_CLOSED_LEVEL_LOW) || (level && sw->closedLevel == SWITCH_CLOSED_LEVEL_HIGH));
}

void Switch_Init(Switch_t* sw, const PinDef_t* pinDef, Switch_ClosedLevel_t closedLevel)
{
	sw->address = GPIO_GetBitBandIOAddress(pinDef);
	sw->currentState = false;
	sw->previousState = false;
	sw->closedLevel = closedLevel;
}
