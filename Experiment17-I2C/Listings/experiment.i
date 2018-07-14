#line 1 "experiment.c"

#line 1 "hal/hal.h"









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






 
#line 11 "hal/hal.h"
#line 1 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdbool.h"
 






 





#line 25 "C:\\Keil_v5\\ARM\\ARMCC\\Bin\\..\\include\\stdbool.h"



#line 12 "hal/hal.h"


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





#line 59 "hal/hal.h"


#line 1 "hal/HAL_PLL.h"









#line 11 "hal/HAL_PLL.h"
#line 1 ".\\startup_TM4C1294\\HAL_Config.h"





#line 12 "hal/HAL_PLL.h"


extern uint32_t PLL_BusClockFreq;


extern uint32_t PLL_NumTicksPer10ms;
extern uint32_t PLL_NumTicksPer10us;
extern uint32_t PLL_NumTicksPer100ns;

#line 28 "hal/HAL_PLL.h"



void PLL_Init120MHz(void);




















void PLL_Init(void);










#line 62 "hal/hal.h"
#line 1 "hal/HAL_SysTick.h"









#line 11 "hal/HAL_SysTick.h"





int SysTick_Init(void);





void SysTick_Wait(uint32_t delay);





void SysTick_Wait10ms(uint32_t delay);





void SysTick_Wait10us(uint32_t delay);





#line 63 "hal/hal.h"
#line 1 "hal/HAL_GPIO.h"










#line 12 "hal/HAL_GPIO.h"


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


#line 64 "hal/hal.h"
#line 1 "hal/HAL_NVIC.h"



#line 5 "hal/HAL_NVIC.h"

void NVIC_EnableIRQ(uint8_t irq, uint8_t priority);
void NVIC_DisableIRQ(uint8_t irq);

#line 65 "hal/hal.h"
#line 1 "hal/HAL_I2C.h"



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
int I2C_MasterWrite(I2C_Module_t module, uint8_t slaveAddress, char* data, int size);
uint8_t I2C_SlaveReadByte(I2C_Module_t module);
int I2C_EnableSlaveDataInterrupt(I2C_Module_t module, uint8_t priority, PFN_I2CRxCallback rxCallback, PFN_I2CTxCallback txCallback);

#line 66 "hal/hal.h"




#line 3 "experiment.c"

static volatile uint32_t* leds_[4];
static  volatile uint32_t* buttons_[2];

static void ReceiveCallback(I2C_Module_t module, uint8_t data, _Bool firstByteReceived)
{
		for (int i = 0; i < 4; i++) {
			*leds_[i] = 0x01 & (data >> i);
		}	
}

static I2C_Module_t i2cModule_;

static int InitHardware()
{
	
		__disable_irq();
	

	
	PLL_Init120MHz();
	
	i2cModule_ = I2C2;
	
#line 33 "experiment.c"
	
	SysTick_Init();
	
	GPIO_EnableDI(PORTJ, PIN0 | PIN1, PULL_UP);
	
	buttons_[0] = GPIO_GetBitBandIOAddress(&((PinDef_t){ PORTJ, PIN0 }));
	buttons_[1] = GPIO_GetBitBandIOAddress(&((PinDef_t){ PORTJ, PIN1 }));
	
	GPIO_EnableDO(PORTN, PIN0 | PIN1, DRIVE_2MA, PULL_DOWN);
	GPIO_EnableDO(PORTF, PIN0 | PIN4, DRIVE_2MA, PULL_DOWN);
	
	leds_[0] = GPIO_GetBitBandIOAddress(&((PinDef_t){ PORTN, PIN1 }));
	leds_[1] = GPIO_GetBitBandIOAddress(&((PinDef_t){ PORTN, PIN0 }));
	leds_[2] = GPIO_GetBitBandIOAddress(&((PinDef_t){ PORTF, PIN4 }));
	leds_[3] = GPIO_GetBitBandIOAddress(&((PinDef_t){ PORTF, PIN0 }));
	
	
	
	I2C_InitModule(i2cModule_);
	I2C_EnableAsMaster(i2cModule_, 100000, 0);


	
	__enable_irq();
	
	return 0;
}



int run(void)
{

	InitHardware();
	
	char data[] = { 0x1, 0x2, 0x4, 0x8 };
	int dataIndex = 0;
	
	for (;;) {
		
		I2C_MasterWrite(i2cModule_, 100, &data[dataIndex], 1); 
		
		dataIndex = (dataIndex + 1) % 4;
		
		SysTick_Wait10ms(25);
		
	}
	
}
