// File:  HAL_Flash.h
// Author: JSpicer
// Date:  12/03/17
// Purpose: Flash utilities
// Hardware:  TM4C123 Tiva board

#ifndef FLASH_HAL_H
#define FLASH_HAL_H

void Flash_Enable(void);
int Flash_Erase(int blockCount);
int Flash_Write(const uint32_t* data, int wordCount);
void Flash_Read(uint32_t* data, int wordCount);

#endif