#ifndef SWITCH5POSITION_H
#define SWITCH5POSITION_H

#include "HAL.h"

typedef enum {
	SWITCH5_CENTER = 1,
	SWITCH5_UP = 2,
	SWITCH5_DOWN = 4,
	SWITCH5_LEFT = 8,
	SWITCH5_RIGHT = 16,
} Switch5Position_t;


typedef struct {
	PinDef_t		centerPin;
	PinDef_t		upPin;
	PinDef_t		downPin;
	PinDef_t		leftPin;
	PinDef_t		rightPin;
} Switch5Config_t;


#define SWITCH5_ISCLOSED(state, position)		((state) & (position))
#define SWITCH5_HASCLOSED(state, position)	(((state) >> 8) & (position))
#define SWITCH5_HASOPENED(state, position)	(((state) >> 16) & (position))


int Switch5_Initialize(const Switch5Config_t* config);
uint32_t Switch5_Poll(void);

#endif
