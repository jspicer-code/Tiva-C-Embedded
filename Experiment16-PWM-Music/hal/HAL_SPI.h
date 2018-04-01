// File:  HAL_SPI.h
// Author: JSpicer
// Date:  11/11/17
// Purpose: SPI utilities
// Hardware:  TM4C123 Tiva board

#ifndef SPI_HAL_H
#define SPI_HAL_H

#include <stdint.h>

// The names of the SSI modules
typedef enum {
	SSI0 = 0,
	SSI1 = 1,
	SSI2 = 2,
	SSI3 = 3
} SSIModule_t;

//----------------------- SPI_Enable --------------------------
// Enables the specified SSI module/channel for SPI
// Inputs:  module - the SSI module name.
// Outputs:  none.
int SPI_Enable(SSIModule_t module);

//----------------------- SPI_Write ---------------------------
// Writes a byte of data to the SPI bus.
// Inputs:  module - the SSI module name.
//          data - the data byte to write.
// Outputs:  none.
void SPI_Write(SSIModule_t module, uint8_t data);

#endif
