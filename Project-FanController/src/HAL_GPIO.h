// File:  HAL_GPIO.h
// Author: JSpicer
// Date:  9/14/17
// Purpose: GPIO hardware abstraction layer.  The functions here
//	can be used for configuing digital and analog I/O.
// Hardware:  TM4C123 Tiva board

#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>

// Names for output drive strength
typedef enum DO_Drive { DRIVE_2MA, DRIVE_4MA, DRIVE_8MA } DO_Drive_t;

// Names for the internal pull-up resistor types.
typedef enum DI_Pull { PULL_NONE, PULL_UP, PULL_DOWN } DI_Pull_t;

//------------------------- GPIO_InitPort ----------------------------
// Initializes a GPIO port for IO usage.
// Inputs:  port - the name of the port to initialize.
// Outputs:  none.
void GPIO_InitPort(PortName_t port);

//------------------------- GPIO_EnableDO ----------------------------
// Enables a set of pins on a GPIO port for digital output.
// Inputs:  port - the name of the port to initialize.
//	        pinMap - a map of pins to enable.
//          drive - the output drive strength to configure
//          pull - the internal pull-up resistor type to configure.
// Outputs:  none.
void GPIO_EnableDO(PortName_t port, uint8_t pinMap, DO_Drive_t drive, DI_Pull_t pull);

//------------------------- GPIO_EnableDI ----------------------------
// Enables a set of pins on a GPIO port for digital input.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
//          pull - the internal pull-up resistor type to configure.
// Outputs:  none.
void GPIO_EnableDI(PortName_t port, uint8_t pinMap, DI_Pull_t pull);


//------------------------- GPIO_EnableAltDigital --------------------
// Enables a set of pins on a GPIO port for an alternate digital function.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
//          ctl - the alternate digital function id (see datasheet).
// Outputs:  none.
void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl);

//------------------------- GPIO_EnableAltAnalog --------------------
// Enables a set of pins on a GPIO port for analog usage.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
// Outputs:  none.
void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap);

//------------------------- GPIO_GetBitBandIOAddress( --------------------
// Gets the bit-band IO address for a pin definition.
// Inputs:  pinDef - pin definition
// Outputs:  pointer with value of bit band address.
volatile uint32_t* GPIO_GetBitBandIOAddress(PinDef_t pinDef);

#endif
