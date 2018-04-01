// File:  HAL_Flash.h
// Author: JSpicer
// Date:  12/03/17
// Purpose: Flash utilities
// Hardware:  TM4C123 Tiva board

#ifndef FLASH_HAL_H
#define FLASH_HAL_H

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
int Flash_Erase(int blockCount);

//------------------------- Flash_Write ----------------------------
// Writes data to Flash memory
// Inputs:  data - a pointer to the data to write.  Must be a multiple
//            of words in size.
// Outputs: wordCount - the number of words of data to write.
int Flash_Write(const void* data, int wordCount);

//------------------------- Flash_Write ----------------------------
// Reads data from Flash memory
// Inputs:  data - a pointer to the data buffer where Flash memory will
//            be copied.
// Outputs: wordCount - the number of words of data to read.
void Flash_Read(void* data, int wordCount);

#endif
