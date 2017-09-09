// File:  GPIO_Util.h
// Author: JS
// Date:  9/9/17
// Purpose: GPIO utilities
// Hardware:  TM4C123 Tiva board

#define PA2 (*((volatile uint32_t*)0x42087F88)) 
#define PA3 (*((volatile uint32_t*)0x42087F8C)) 
#define PA4 (*((volatile uint32_t*)0x42087F90)) 
#define PA5 (*((volatile uint32_t*)0x42087F94)) 
#define PA6 (*((volatile uint32_t*)0x42087F98)) 
#define PA7 (*((volatile uint32_t*)0x42087F9C)) 

#define PE0 (*((volatile uint32_t*)0x42487F80))
#define PE1 (*((volatile uint32_t*)0x42487F84))
#define PE2 (*((volatile uint32_t*)0x42487F88))
#define PE3 (*((volatile uint32_t*)0x42487F8C))
#define PE4 (*((volatile uint32_t*)0x42487F90))
#define PE5 (*((volatile uint32_t*)0x42487F94))

#define PF0 (*((volatile uint32_t*)0x424A7F80))
#define PF1 (*((volatile uint32_t*)0x424A7F84))
#define PF2 (*((volatile uint32_t*)0x424A7F88))
#define PF3 (*((volatile uint32_t*)0x424A7F8C))
#define PF4 (*((volatile uint32_t*)0x424A7F90))

#define PIN_0		0x01
#define PIN_1		0x02
#define PIN_2		0x04
#define PIN_3		0x08
#define PIN_4		0x10
#define PIN_5		0x20
#define PIN_6		0x40
#define PIN_7		0x80


typedef enum DO_Drive { DRIVE_2MA, DRIVE_4MA, DRIVE_8MA } DO_Drive_t;
typedef enum DI_Pull { PULL_NONE, PULL_UP, PULL_DOWN } DI_Pull_t;

void GPIO_PortA_Init(void);
void GPIO_PortA_EnableDO(uint8_t pinMap, DO_Drive_t drive);
void GPIO_PortA_EnableDI(uint8_t pinMap, DI_Pull_t pull);

void GPIO_PortE_Init(void);
void GPIO_PortE_EnableDO(uint8_t pinMap, DO_Drive_t drive);
void GPIO_PortE_EnableDI(uint8_t pinMap, DI_Pull_t pull);

void GPIO_PortF_Init(void);
void GPIO_PortF_EnableDO(uint8_t pinMap, DO_Drive_t drive);
void GPIO_PortF_EnableDI(uint8_t pinMap, DI_Pull_t pull);

