// File:  HAL_I2C.h
// Author: JS
// Date:  7/14/18
// Purpose: I2C utilities
// Hardware:  TM4C123 and TM4C1294 Tiva board

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "HAL_Config.h"

typedef enum { 
	I2C0 = 0, 
	I2C1 = 1, 
	I2C2 = 2, 
	I2C3 = 3,
#if (halCONFIG_1294 == 1)
	I2C4 = 4,
	I2C5 = 5,
	I2C6 = 6,
	I2C7 = 7,
	I2C8 = 8,
	I2C9 = 9
#endif
} I2C_Module_t;

// Signature of the I2C callback function 
typedef void (*PFN_I2CRxCallback)(I2C_Module_t module, uint8_t data, bool firstByteReceived);
typedef void (*PFN_I2CTxCallback)(I2C_Module_t module, uint8_t* data);

// Configures the clock gating and IO pins for an I2C module.
// Inputs:  module - the I2C module name
// Output:  zero indicates success
int I2C_InitModule(I2C_Module_t module);

// Enables an I2C module for master function.
// Note:  High-speed mode has not been tested.
// Inputs:  module - the I2C module name
//					speed - bus speed, e.g 100000=Standard, 400000=Fast, 1000000=Fast Plus
// 					loopbackMode - enables loopback mode
// Output: none
void I2C_EnableAsMaster(I2C_Module_t module, uint32_t speed, bool loopbackMode);

// Enables an I2C module for slave function.
// Inputs:	module - the I2C module name
//					ownAddress - the slave address to use.
// Output:	none
void I2C_EnableAsSlave(I2C_Module_t module, uint8_t ownAddress);

// Starts a bus transmit operation.
// Inputs:	module - the I2C module name
//					slaveAddress - the receiving slave address
//					data - array of data to write
//					size - the number of bytes in the data array to write
//					repeatedStart - indicates whether this write operation is part of an ongoing transaction.
//						If repeatedStart is false, the function waits for the bus to become idle before transacting, 
//						otherwise it assumes it has ownership and does not wait.
//					generateStop - generates a stop bit at the end of the transaction.  Set to false if
//						a repeated start will be issue next.
// Output: zero if success, non-zero if an error.  TODO:  The bits in the return value are not currently
//						defined for the caller.
int I2C_MasterWrite(I2C_Module_t module, uint8_t slaveAddress, char* data, int size, bool repeatedStart, bool generateStop);

// Starts a bus receive operation.
// Inputs:	module - the I2C module name
//					slaveAddress - the targeted slave address
//					data - buffer to hold data read from slave
//					size - the number of bytes to read from the slave.
//					repeatedStart - indicates whether this read operation is part of an ongoing transaction.
//						If repeatedStart is false, the function waits for the bus to become idle before transacting, 
//						otherwise it assumes it has ownership and does not wait.
//					generateStop - generates a stop bit at the end of the transaction.  Set to false if
//						a repeated start will be issue next.
// Output: zero if success, non-zero if an error.  TODO:  The bits in the return value are not currently
//						defined for the caller. 
int I2C_MasterRead(I2C_Module_t module, uint8_t slaveAddress, char* data, int size, bool repeatedStart, bool generateStop);

// This function implements a blocking read of one byte.  It can be used for loopback testing.
uint8_t I2C_SlaveReadByte(I2C_Module_t module);

// Enables the interrupt handlers for slsave receive and transmit operations.
// Inputs:  module - the I2C module name
//					priority - interrupt priority 0-7
//					rxCallback - callback function invoked when data for the slave is received.
//					txCallback - callback function invoked when data from the slave is requested.
// Output:  zero if success
int I2C_EnableSlaveDataInterrupt(I2C_Module_t module, uint8_t priority, PFN_I2CRxCallback rxCallback, PFN_I2CTxCallback txCallback);

#endif
