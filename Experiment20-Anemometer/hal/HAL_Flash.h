// File:  HAL_Flash.h
// Author: JSpicer
// Date:  12/03/17
// Purpose: Flash utilities
// Hardware:  TM4C123 Tiva board

#ifndef FLASH_HAL_H
#define FLASH_HAL_H

// The TM4C123 has 256 KiB of Flash memory located from addresses 0 to 0x0003FFFF
//	Make sure that the base address used here does not overlap with code and data!
//	Examine the linker's .map file to see where code/data is stored.

#define FLASH_BLOCK_SIZE_WORDS	256

//------------------------- Flash_Enable --------------------------
// Enables the Flash HAL module.  Must be called once prior to Erase
//	Write, and Read operations.
// Inputs:  none.
// Outputs:  none.
void Flash_Enable(void);

//------------------------- Flash_Erase ----------------------------
// Erases a block for Flash memory
// Inputs:  blockCount - the number of blocks to erase.
// Outputs:  none.
int Flash_Erase(volatile uint32_t* address, int blockCount);

//------------------------- Flash_Write ----------------------------
// Writes data to Flash memory
// Inputs:  data - a pointer to the data to write.  Must be a multiple
//            of words in size.
//          wordCount - the number of words of data to write.
int Flash_Write(volatile uint32_t* address, const void* data, int wordCount);

//------------------------- Flash_Write ----------------------------
// Reads data from Flash memory
// Inputs:  data - a pointer to the data buffer where Flash memory will
//            be copied.
//          wordCount - the number of words of data to read.
void Flash_Read(volatile uint32_t* address, void* data, int wordCount);

#endif
