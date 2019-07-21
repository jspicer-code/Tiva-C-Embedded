#line 1 "drivers\\Switch.c"






#line 1 "drivers\\Switch.h"









#line 1 ".\\hal\\HAL.h"









#line 1 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdint.h"
 
 





 









     
#line 27 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdint.h"
     











#line 46 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdint.h"





 

     

     
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;
typedef   signed       __int64 int64_t;

     
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;
typedef unsigned       __int64 uint64_t;

     

     
     
typedef   signed          char int_least8_t;
typedef   signed short     int int_least16_t;
typedef   signed           int int_least32_t;
typedef   signed       __int64 int_least64_t;

     
typedef unsigned          char uint_least8_t;
typedef unsigned short     int uint_least16_t;
typedef unsigned           int uint_least32_t;
typedef unsigned       __int64 uint_least64_t;

     

     
typedef   signed           int int_fast8_t;
typedef   signed           int int_fast16_t;
typedef   signed           int int_fast32_t;
typedef   signed       __int64 int_fast64_t;

     
typedef unsigned           int uint_fast8_t;
typedef unsigned           int uint_fast16_t;
typedef unsigned           int uint_fast32_t;
typedef unsigned       __int64 uint_fast64_t;

     




typedef   signed           int intptr_t;
typedef unsigned           int uintptr_t;


     
typedef   signed     long long intmax_t;
typedef unsigned     long long uintmax_t;




     

     





     





     





     

     





     





     





     

     





     





     





     

     






     






     






     

     


     


     


     

     
#line 216 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdint.h"

     



     






     
    
 



#line 241 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdint.h"

     







     










     











#line 305 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdint.h"






 
#line 11 ".\\hal\\HAL.h"
#line 1 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdbool.h"
 






 





#line 25 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdbool.h"



#line 12 ".\\hal\\HAL.h"


typedef enum { 
	
	
	PORTA = 0, 
	PORTB = 1, 
	PORTC = 2, 
	PORTD = 3, 
	PORTE = 4, 
	PORTF = 5,

	
	PORTG = 6,	
	PORTH = 7,
	PORTJ = 8,
	PORTK = 9,
	PORTL = 10,
	PORTM = 11,
	PORTN = 12,
	PORTP = 13,
	PORTQ = 14
} PortName_t;	



typedef enum { 
	PIN0 = 0x01,
	PIN1 = 0x02,
	PIN2 = 0x04,
	PIN3 = 0x08,
	PIN4 = 0x10,
	PIN5 = 0x20,
	PIN6 = 0x40,
	PIN7 = 0x80
} PinName_t;	


typedef struct {
	PortName_t		port;
	PinName_t			pin;
}	PinDef_t;




#line 1 ".\\startup_TM4C1294\\HAL_Config.h"






















#line 59 ".\\hal\\HAL.h"


#line 1 ".\\hal\\HAL_PLL.h"









#line 11 ".\\hal\\HAL_PLL.h"
#line 1 ".\\startup_TM4C1294\\HAL_Config.h"






















#line 12 ".\\hal\\HAL_PLL.h"

typedef enum { 
	
	SYSCLK_80,
	

	SYSCLK_120 

	
} SysClkFreq_t;


extern uint32_t PLL_BusClockFreq;


extern uint32_t PLL_NumTicksPer10ms;
extern uint32_t PLL_NumTicksPer10us;
extern uint32_t PLL_NumTicksPer100ns;





void PLL_Init(SysClkFreq_t freq);





#line 62 ".\\hal\\HAL.h"
#line 1 ".\\hal\\HAL_SysTick.h"









#line 11 ".\\hal\\HAL_SysTick.h"





int SysTick_Init(void);





void SysTick_Wait(uint32_t delay);





void SysTick_Wait10ms(uint32_t delay);





void SysTick_Wait10us(uint32_t delay);





#line 63 ".\\hal\\HAL.h"
#line 1 ".\\hal\\HAL_GPIO.h"










#line 12 ".\\hal\\HAL_GPIO.h"
#line 1 ".\\hal\\HAL.h"






#line 70 ".\\hal\\HAL.h"


#line 13 ".\\hal\\HAL_GPIO.h"


typedef enum { DRIVE_2MA, DRIVE_4MA, DRIVE_8MA } GPIO_Drive_t;


typedef enum { PULL_NONE, PULL_UP, PULL_DOWN } GPIO_Pull_t;


typedef enum { 
	INT_TRIGGER_NONE, 
	INT_TRIGGER_LOW_LEVEL,
	INT_TRIGGER_HIGH_LEVEL,
	INT_TRIGGER_RISING_EDGE,
	INT_TRIGGER_FALLING_EDGE,
	INT_TRIGGER_BOTH_EDGES
} GPIO_IntTrigger_t;


typedef void (*PFN_GPIOCallback)(PortName_t port, uint32_t pinMap);





void GPIO_InitPort(PortName_t port);








void GPIO_EnableDO(PortName_t port, uint8_t pinMap, GPIO_Drive_t drive, GPIO_Pull_t pull);







void GPIO_EnableDI(PortName_t port, uint8_t pinMap, GPIO_Pull_t pull);








void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl, _Bool openDrain);






void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap);





volatile uint32_t* GPIO_GetBitBandIOAddress(const PinDef_t* pinDef);





int GPIO_EnableInterrupt(const PinDef_t* pinDef, uint8_t priority, GPIO_IntTrigger_t trigger, PFN_GPIOCallback callback);





void GPIO_DisarmInterrupt(const PinDef_t* pinDef); 





void GPIO_RearmInterrupt(const PinDef_t* pinDef); 


#line 64 ".\\hal\\HAL.h"
#line 1 ".\\hal\\HAL_NVIC.h"









#line 11 ".\\hal\\HAL_NVIC.h"

void NVIC_EnableIRQ(uint8_t irq, uint8_t priority);
void NVIC_DisableIRQ(uint8_t irq);

#line 65 ".\\hal\\HAL.h"
#line 1 ".\\hal\\HAL_I2C.h"









#line 11 ".\\hal\\HAL_I2C.h"
#line 12 ".\\hal\\HAL_I2C.h"
#line 1 ".\\startup_TM4C1294\\HAL_Config.h"






















#line 13 ".\\hal\\HAL_I2C.h"

typedef enum { 
	I2C0 = 0, 
	I2C1 = 1, 
	I2C2 = 2, 
	I2C3 = 3,

	I2C4 = 4,
	I2C5 = 5,
	I2C6 = 6,
	I2C7 = 7,
	I2C8 = 8,
	I2C9 = 9

} I2C_Module_t;


typedef void (*PFN_I2CRxCallback)(I2C_Module_t module, uint8_t data, _Bool firstByteReceived);
typedef void (*PFN_I2CTxCallback)(I2C_Module_t module, uint8_t* data);




int I2C_InitModule(I2C_Module_t module);







void I2C_EnableAsMaster(I2C_Module_t module, uint32_t speed, _Bool loopbackMode);





void I2C_EnableAsSlave(I2C_Module_t module, uint8_t ownAddress);













int I2C_MasterWrite(I2C_Module_t module, uint8_t slaveAddress, char* data, int size, _Bool repeatedStart, _Bool generateStop);













int I2C_MasterRead(I2C_Module_t module, uint8_t slaveAddress, char* data, int size, _Bool repeatedStart, _Bool generateStop);


uint8_t I2C_SlaveReadByte(I2C_Module_t module);







int I2C_EnableSlaveDataInterrupt(I2C_Module_t module, uint8_t priority, PFN_I2CRxCallback rxCallback, PFN_I2CTxCallback txCallback);

#line 66 ".\\hal\\HAL.h"
#line 1 ".\\hal\\HAL_Timer.h"









#line 11 ".\\hal\\HAL_Timer.h"
#line 12 ".\\hal\\HAL_Timer.h"


typedef enum {
	TIMER0 = 0,
	TIMER1 = 1,
	TIMER2 = 2,
	TIMER3 = 3,
	TIMER4 = 4,
	TIMER5 = 5,

	TIMER6 = 6,
	TIMER7 = 7,

} TimerBlock_t;


typedef enum {
	TIMER_ONESHOT,
	TIMER_PERIODIC,
	TIMER_EDGE_TIMER,
	TIMER_COUNTER
} TimerMode_t;


typedef enum {
	TIMER_NO_EVENT = 0,
	TIMER_TIMEOUT_EVENT,
	TIMER_EDGE_EVENT
} TimerEventType_t;


typedef struct {
	TimerBlock_t block;
	void* callbackData;
	TimerEventType_t eventType;
	union {
		uint32_t timerCount;
	} eventData;
} TimerEventArgs_t;


typedef void (*PFN_TimerCallback)(TimerBlock_t block, const TimerEventArgs_t* args);

typedef struct {
	uint8_t priority;
	PFN_TimerCallback callback;
	void* callbackData;
} TimerIRQConfig_t;











int Timer_Init(TimerBlock_t block, TimerMode_t mode, const TimerIRQConfig_t* irqConfig, const PinDef_t* pinConfig);

void Timer_Stop(TimerBlock_t block);








int Timer_StartInterval(TimerBlock_t block, uint32_t interval);







void Timer_Wait(TimerBlock_t block, uint32_t count, uint32_t interval);






int Timer_StartEdgeTimer(TimerBlock_t block);



#line 123 ".\\hal\\HAL_Timer.h"



#line 67 ".\\hal\\HAL.h"
#line 1 ".\\hal\\HAL_SPI.h"









#line 11 ".\\hal\\HAL_SPI.h"


typedef enum {
	SSI0 = 0,
	SSI1 = 1,
	SSI2 = 2,
	SSI3 = 3
} SSIModule_t;

typedef enum {
	SSI_DATASIZE_8,
	SSI_DATASIZE_16
} SSIDataSize_t;





int SPI_Enable(SSIModule_t module, SSIDataSize_t dataSize);






void SPI_Write(SSIModule_t module, uint16_t data);

#line 68 ".\\hal\\HAL.h"




#line 11 "drivers\\Switch.h"





typedef enum {	
	SWITCH_CLOSED_LEVEL_HIGH,
	SWITCH_CLOSED_LEVEL_LOW
} Switch_ClosedLevel_t;	
	
typedef struct {	
	volatile uint32_t* address;
	Switch_ClosedLevel_t closedLevel;
	_Bool currentState;
	_Bool previousState;
} Switch_t;

void Switch_Init(Switch_t* sw, const PinDef_t* pinDef, Switch_ClosedLevel_t closedLevel);
void Switch_Poll(Switch_t* sw);

#line 8 "drivers\\Switch.c"

void Switch_Poll(Switch_t* sw)
{
	sw->previousState = sw->currentState;
	uint8_t level = *sw->address;
	sw->currentState = ((!level && sw->closedLevel == SWITCH_CLOSED_LEVEL_LOW) || (level && sw->closedLevel == SWITCH_CLOSED_LEVEL_HIGH));
}

void Switch_Init(Switch_t* sw, const PinDef_t* pinDef, Switch_ClosedLevel_t closedLevel)
{
	sw->address = GPIO_GetBitBandIOAddress(pinDef);
	sw->currentState = 0;
	sw->previousState = 0;
	sw->closedLevel = closedLevel;
}
