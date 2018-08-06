// File:  HAL_I2C.c
// Author: JS
// Date:  7/14/18
// Purpose: I2C utilities
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include <assert.h>
#include "HAL.h"

#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else  // TM4C123
#include "tm4c123gh6pm.h"
#endif

#if (halCONFIG_1294 == 1)
#define I2C_MAX_MODULES		10
#else  // TM4C123
#define I2C_MAX_MODULES		4
#endif


// This structure represents the registers associated with the I2C module.
//	It will be overlayed on top of IO memory so that the structure fields
//	map to the registers.  (See the datasheet for field/register descriptions).
typedef struct {
  
	uint32_t  MSA;
	uint32_t  MCS;
	uint32_t  MDR;
  uint32_t  MTPR;
  uint32_t  MIMR;
  uint32_t  MRIS;
  uint32_t  MMIS;
  uint32_t  MICR;
  uint32_t  MCR;
  uint32_t  MCLKOCNT;
  uint32_t  RESERVED0;
  uint32_t  MBMON;

#if (halCONFIG_1294 == 1)
  uint32_t  MBLEN;
  uint32_t  MBCNT;
	uint32_t  RESERVED1[498];
#else // TM4C123
	uint32_t 	RESERVED1[2];
	uint32_t	MCR2;
	uint32_t	RESERVED2[497];
#endif

	uint32_t  SOAR;
  uint32_t  SCSR;
  uint32_t  SDR;
  uint32_t  SIMR;
  uint32_t  SRIS;
  uint32_t  SMIS;
  uint32_t  SICR;
  uint32_t  SOAR2;
  uint32_t  SACKCTL;
 
#if (halCONFIG_1294 == 1)
 	uint32_t  RESERVED4[439]; 
	uint32_t  FIFODATA;
  uint32_t  FIFOCTL;
  uint32_t  FIFOSTATUS;
  uint32_t  RESERVED5[45];
#else // TM4C123
	uint32_t	RESERVED3[487];
#endif

  uint32_t  PP;
  uint32_t  PC;
	
} I2CRegs_t;


// These are base IO addresses of the I2C modules.
#define I2C0_REG_BASE 	((volatile uint32_t *)0x40020000)
#define I2C1_REG_BASE		((volatile uint32_t *)0x40021000)
#define I2C2_REG_BASE		((volatile uint32_t *)0x40022000)
#define I2C3_REG_BASE		((volatile uint32_t *)0x40023000)
#define I2C4_REG_BASE		((volatile uint32_t *)0x400C0000)
#define I2C5_REG_BASE		((volatile uint32_t *)0x400C1000)
#define I2C6_REG_BASE		((volatile uint32_t *)0x400C2000)
#define I2C7_REG_BASE		((volatile uint32_t *)0x400C3000)
#define I2C8_REG_BASE		((volatile uint32_t *)0x400B8000)
#define I2C9_REG_BASE		((volatile uint32_t *)0x400B9000)

// This array is a look table to resolve the I2C module name to its base address.
const volatile uint32_t * I2CBaseAddress_[I2C_MAX_MODULES] = {
	I2C0_REG_BASE,
	I2C1_REG_BASE,
	I2C2_REG_BASE,
	I2C3_REG_BASE,
#if (halCONFIG_1294 == 1)
	I2C4_REG_BASE,
	I2C5_REG_BASE,
	I2C6_REG_BASE,
	I2C7_REG_BASE,
	I2C8_REG_BASE,
	I2C9_REG_BASE
#endif
};

#define DEFINE_IRQ_HANDLER(name) \
static PFN_I2CRxCallback name##_RxCallback; \
static PFN_I2CTxCallback name##_TxCallback; \
void I2C_##name##Handler(void) \
{ \
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[name]; \
	HandleInterrupt(i2c, name, name##_RxCallback, name##_TxCallback); \
}

#define ENABLE_IRQ_HANDLER(name, priority, rxCallback, txCallback) \
{ \
	NVIC_EnableIRQ(INT_##name - 16, priority); \
	name##_RxCallback = rxCallback; \
	name##_TxCallback = txCallback; \
}

// Forward declaration, defined at the bottom of this file.
static void HandleInterrupt(volatile I2CRegs_t* i2c, I2C_Module_t module, 
	PFN_I2CRxCallback rxCallback, PFN_I2CTxCallback txCallback);

#if (halUSE_I2C0 == 1)
// I2C0 interrupt handler.  
// INT# 24, IRQ 8.  Same for both TM4C123 and TM4C1294.
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(I2C0)
#endif

#if (halUSE_I2C2 == 1)
// I2C2 interrupt handler.  
// TM4C123: INT# 84, IRQ 68
// TM4C1294:  INT# 77, IRQ 61
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(I2C2)
#endif


int I2C_InitModule(I2C_Module_t module)
{
	uint32_t moduleBit = (0x1 << (uint8_t)module);
	
	if (!(SYSCTL_PRI2C_R & moduleBit)) {
	
		// Clock gating control register - provide I2C module with a clock.
		SYSCTL_RCGCI2C_R |= moduleBit; 
	
		// Perpheral ready register - wait until the correponding module bit is set (enabled).
		while (!(SYSCTL_PRI2C_R & moduleBit )){}
	}
		
	// Enable GPIO pins for alternate function I2C
	switch (module) {
		
#if (halUSE_I2C0 == 1)
		case I2C0:
			// The TM4C123 and TM4C124 use different PCTL values.
			// Enable open-drain for SCL but not SDA.
#if (halCONFIG_1294 == 1)
			GPIO_EnableAltDigital(PORTB, PIN2, 0x2, false		// SCL
			GPIO_EnableAltDigital(PORTB, PIN3, 0x2, true);	// SDA (open-drain)
#else // TM4C123
			GPIO_EnableAltDigital(PORTB, PIN2, 0x3, false);	// SCL
			GPIO_EnableAltDigital(PORTB, PIN3, 0x3, true);	// SDA (open-drain)		
#endif
			break;
#endif

#if (halUSE_I2C2 == 1)
		case I2C2:
			// The TM4C123 and TM4C124 use different PCTL values.
			// Enable open-drain for SCL but not SDA.
#if (halCONFIG_1294 == 1)
			GPIO_EnableAltDigital(PORTN, PIN5, 0x3, false);	// SCL
			GPIO_EnableAltDigital(PORTN, PIN4, 0x3, true);	// SDA (open-drain)
#else // TM4C123
			GPIO_EnableAltDigital(PORTE, PIN4, 0x3, false);	// SCL
			GPIO_EnableAltDigital(PORTE, PIN5, 0x3, true);	// SDA (open-drain)		
#endif
			break;
#endif
		
		default:
			abort();  // Enable other modules as needed.
			break;
	}

	return 0;
	
}

void I2C_EnableAsMaster(I2C_Module_t module, uint32_t speed, bool loopbackMode)
{
	// Overlay the I2C register structure on top of the I2Cx memory region...
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[module];

	// Enable master mode.
	i2c->MCR |= I2C_MCR_MFE;
		
	/// Set the I2C clock period...
	// CLK_PRD = 1/BusClockFreq
	// SCL_LP = 6 (fixed)
	// SCL_HP = 4 (fixed
	// SCL_PERIOD =  2 * (1 + MTPR) * (SCL_LP + SCL_HP) * CLK_PRD = 1/speed
	// => MTPR = SCL_PERIOD/((2*(6+4)*CLK_PRD) - 1
	// => MTPR = BusClockFreq/(20*speed) - 1
	uint32_t tpr = PLL_BusClockFreq / (20 * speed) - 1;
	i2c->MTPR = (i2c->MTPR & ~I2C_MTPR_TPR_M) | (tpr & I2C_MTPR_TPR_M);
	
	if (loopbackMode) {
		i2c->MCR |= I2C_MCR_LPBK;
	}
	else {
		i2c->MCR &= ~I2C_MCR_LPBK;		
	}
	
}


void I2C_EnableAsSlave(I2C_Module_t module, uint8_t ownAddress)
{
	// Overlay the I2C register structure on top of the I2Cx memory region...
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[module];
	
	i2c->MCR |= I2C_MCR_SFE;
	i2c->SOAR |= (I2C_SOAR_OAR_M & ownAddress);
	i2c->SCSR |= I2C_SCSR_DA;
	
}


// This function writes a command to the MCS (master command/status) register
//	which starts or continues the transaction.  The datasheet says to wait for
//	the BUSY flag to clear in the MCS after the being written to, but in practice
//	this flag does not set immediately after MCS is written.  The workaround recommended
//  in the TI TM4C forums is to wait for BUSY to set, then wait for it to clear, e.g. 
//	
//	// Wait for the BUSY bit to set...
//	while (!(i2c->MCS & I2C_MCS_BUSY));
//	
//	// Wait for the BUSY bit to clear...
//	do {
//		mcs = i2c->MCS;	
//		// Save the error status flags, since reading MCS seems to clear them and they
//		//	are set before BUSY clears.
//		error |= mcs & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR);
//	} while (mcs & I2C_MCS_BUSY);
//
// However, this is not reliable and sometimes an infinite loop will occur if BUSY
//	does set before reaching the first wait loop.  The alternative solutions given are
//	to introduce a fixed delay, which implies using SysTick, a Timer, or a spin loop.
//	This also has	the potential to be unreliable if the wait duration is not accurate,
//	and SysTick	must be reserved for the RTOS.  
//	
// The solution adopted here is to instead test the RIS bit (raw interrupt status) in
//	the MRIS register, which is set when the next byte to transfer is requested.  So far,
//	this solution is working reliably.
//
static int InvokeMasterCommand(volatile I2CRegs_t* i2c, uint32_t mcs)
{
	int error = 0;

	// Clear the RIS bit (master interrupt)
	i2c->MICR |= I2C_MICR_IC;
	
	// Invoke the command.
	i2c->MCS = mcs;

	// Wait until the RIS bit is set, which indicates the next byte to transfer is being requested.
	while (!(i2c->MRIS & I2C_MRIS_RIS));
	
	// Check the error status.
	mcs = i2c->MCS;
	error |= mcs & (I2C_MCS_ARBLST | I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR);
	
	// If there was an error but it was not Arbitration Lost, then issue a STOP.
	if (error & I2C_MCS_ERROR && !(error & I2C_MCS_ARBLST)) {
		i2c->MCS = I2C_MCS_STOP;
	}
	
	return error;
	
}


int I2C_MasterWrite(I2C_Module_t module, uint8_t slaveAddress, char* data, int size, bool repeatedStart, bool generateStop)
{
	// Overlay the I2C register structure on top of the I2Cx memory region...
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[module];

	// Set the slave address.  The R/S bit (0) is cleared for a write.
	i2c->MSA = (i2c->MSA & ~0xFF) | (slaveAddress << 1);
	
	int error = 0;
	
	// Loop until all data is written or an error occurs.
	for (int i = 0; i < size && !error; i++) {
	
		// Note: data in MDR reads back as zero in the debugger.
		i2c->MDR = data[i];

		uint32_t mcs = I2C_MCS_RUN;				
	
		if (i == 0) {

			// Wait until the bus is idle unless this is a repeated start, in which case
			// the bus will be busy because a transaction is still in progress.
			if (!repeatedStart) { 
				while (i2c->MCS & I2C_MCS_BUSBSY);
			}		
				
			// START if this is the first byte.
			mcs |= I2C_MCS_START;		
		}

		// STOP if this is the last byte.
		if (generateStop && (i == size - 1)) {
			mcs |= I2C_MCS_STOP;
		}
				
		error = InvokeMasterCommand(i2c, mcs);
		
	}
			
	return error;
		
}
	
	
int I2C_MasterRead(I2C_Module_t module, uint8_t slaveAddress, char* data, int size, bool repeatedStart, bool generateStop)
{

	// Overlay the I2C register structure on top of the I2Cx memory region...
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[module];

	// Set the slave address.  Set the R/S bit for a read.
	i2c->MSA = (i2c->MSA & ~0xFF) | ((slaveAddress << 1) | I2C_MSA_RS);
	
	int error = 0;
	
	// Loop until the expected number of data bytes is read or an error (e.g. NAK) occurs.
	for (int i = 0; i < size && !error; i++) {

		uint32_t mcs = I2C_MCS_RUN;				
	
		if (i == 0) {

			// Wait until the bus is idle unless this is a repeated start, in which case
			// the bus will be busy because a transaction is still in progress.
			if (!repeatedStart) { 
				while (i2c->MCS & I2C_MCS_BUSBSY);
			}
			
			// START if this is the first byte.  ACK to acknowledge slave.
			mcs |= I2C_MCS_START;		
		}

		if (i < size - 1) {
			// ACK all bytes except the last one.
			mcs |= I2C_MCS_ACK;
		}
		else if (generateStop) {
			mcs |= I2C_MCS_STOP;
		}
		
		error = InvokeMasterCommand(i2c, mcs);
		if (!error) {	
			data[i] = i2c->MDR;
		}
		
	}	
		
	return error;
}		
	

// This function implements a blocking read of one byte.  It can be used for loopback testing.
uint8_t I2C_SlaveReadByte(I2C_Module_t module)
{
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[module];

	volatile uint32_t scsr = 0;
	while (!(scsr & I2C_SCSR_RREQ)) {
		scsr = i2c->SCSR; 
	}

	return (uint8_t)(I2C_SDR_DATA_M & i2c->SDR);
	
}

int I2C_EnableSlaveDataInterrupt(I2C_Module_t module, uint8_t priority, 
	PFN_I2CRxCallback rxCallback, PFN_I2CTxCallback txCallback)
{
	
	volatile I2CRegs_t* i2c = (volatile I2CRegs_t*)I2CBaseAddress_[module];
	
	// Clear any prior interrupt and unmask interrupt for data interrupt
	i2c->SICR |= I2C_SICR_DATAIC;
	i2c->SIMR |= I2C_SIMR_DATAIM; 
	
	switch (module) {
	
#if (halUSE_I2C0 == 1)		
		case I2C0:
			ENABLE_IRQ_HANDLER(I2C0, priority, rxCallback, txCallback);
			break;
#endif

#if (halUSE_I2C2 == 1)		
		case I2C2:
			ENABLE_IRQ_HANDLER(I2C2, priority, rxCallback, txCallback);
			break;
#endif
		
		default:
			abort();	 // Enable other modules as needed.
			break;
	}
	
	return 0;
}


// Generic handler (called by the interrupt handles) which clears the interrupt flags 
//	and invokes the user callback as needed.
static void HandleInterrupt(volatile I2CRegs_t* i2c, I2C_Module_t module, 
	PFN_I2CRxCallback rxCallback, PFN_I2CTxCallback txCallback)
{
	volatile uint32_t readback;

	// Is this a data interrupt?
	if (i2c->SMIS & I2C_SMIS_DATAMIS) {
		
		// Clear the interrupt flag
		i2c->SICR |= I2C_SICR_DATAIC;
		
		// Read to force clearing of the interrupt flag.
		readback = i2c->SICR;
		
		// Reading SCSR will clear it, so it must be copied.
		uint32_t scr = i2c->SCSR;
		
		uint8_t data;
		// Check whether this is data to be received or transmitted and call
		//	the appropriate callback.
		if (scr & I2C_SCSR_RREQ && rxCallback) { 
			// Data received
			data = (uint8_t)i2c->SDR;
			rxCallback(module, data, (bool)(scr & I2C_SCSR_FBR));
		}
		else if (scr & I2C_SCSR_TREQ && txCallback) {
			// Data to be transmitted.
			txCallback(module, &data);
			i2c->SDR = data;
		}
	
	}
	
}



