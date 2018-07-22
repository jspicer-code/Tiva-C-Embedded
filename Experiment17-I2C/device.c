// File:  device.c
// Author: JS
// Date:  7/14/18
// Purpose: I2C device module
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "device.h"

// From the NXP spec, the groups 0000 XXX and 1111 XXX are reserved,
//  so number the slaves BASE + id, e.g. 0x9, 0xA, 0xB, ...
#define SLAVE_ADDRESS_BASE 	0x8 // 0001 XXX

#define COMMAND_WRITE  	0xA0
#define COMMAND_READ  	0xA1
#define COMMAND_BLINKY	0xA2

// Buttons are normally HIGH and go LOW when pressed.
#define SW1		0x1
#define SW2		0x2
#define IsButtonDown(state, button) (state & button)
#define WasButtonPressed(state, button) (state & (button << 2))

static volatile uint32_t* colorLeds_[4];
static volatile uint32_t* patternLeds_[4];
static volatile uint32_t* buttons_[2];
static volatile uint32_t* dipSwitches_[8];
static volatile uint32_t* slavePins_[2];

static struct Settings_t {
	uint8_t slaveTargetId;
	uint8_t color;
	uint8_t pattern;
	bool blinkyMode;
} settings_;
	
static struct CommandStatus_t {
	uint8_t command;
	uint8_t bytesReceived;
	uint8_t bytesTransmitted;
} commandStatus_;

static void SetColorLeds(uint8_t color)
{
	switch (color) {
				
		case 0:
			*colorLeds_[0] = 0;
			*colorLeds_[1] = 0;
			*colorLeds_[2] = 0;
			break;
		case 1:
			*colorLeds_[0] = 1;
			*colorLeds_[1] = 0;
			*colorLeds_[2] = 0;
			break;
		case 2:
			*colorLeds_[0] = 0;
			*colorLeds_[1] = 1;
			*colorLeds_[2] = 0;
			break;
		case 3:
			*colorLeds_[0] = 0;
			*colorLeds_[1] = 0;
			*colorLeds_[2] = 1;
			break;
	}
	
}

static void SetLedPattern(uint8_t pattern)
{
	// Assign the rightmost bit to the righmost LED
	for (int i = 0; i < 4; i++) {
		*patternLeds_[3 - i] = 0x01 & (pattern >> i);
	}	
}

static void ReceiveCallback(I2C_Module_t module, uint8_t data, bool firstByteReceived)
{
	
	// The first byte received will be the command byte. Subsequent bytes will be the (payload) data bytes.
	if (firstByteReceived) {
		
		commandStatus_.command = data;
		commandStatus_.bytesReceived = 0;
		commandStatus_.bytesTransmitted = 0;
		
		settings_.blinkyMode = (commandStatus_.command == COMMAND_BLINKY);
		
	}
	else if (commandStatus_.command == COMMAND_WRITE) {
		// Another device is writing its switch data to this device.  Update the LEDs.
		if (commandStatus_.bytesReceived == 0) {
			SetColorLeds(data);
		}		
		else if (commandStatus_.bytesReceived == 1) {
			SetLedPattern(data);
		}

		commandStatus_.bytesReceived++;
	}

}

static void TransmitCallback(I2C_Module_t module, uint8_t* data)
{
	// If the last command byte sent was COMMAND_READ, the master device is requesting to read
	//	this device's switch data.
	if (commandStatus_.command == COMMAND_READ) {
		
		// The first payload byte to transmit is the color, and the next is the LED pattern.
		if (commandStatus_.bytesTransmitted == 0) {
			*data = settings_.color;
		}
		else if (commandStatus_.bytesTransmitted == 1) {
			*data = settings_.pattern;
		}
		
		commandStatus_.bytesTransmitted++;
	}
}

static int InitHardware(DeviceConfig_t* pConfig)
{
	
	__disable_irq();
	
	PLL_Init(pConfig->sysClkFreq);	
	
	SysTick_Init();

	for (int i = 0; i < 2; i++) {
		GPIO_EnableDI(pConfig->slavePins[i].port, pConfig->slavePins[i].pin, PULL_DOWN);
		slavePins_[i] = GPIO_GetBitBandIOAddress(&pConfig->slavePins[i]);
	}
	
	for (int i = 0; i < 2; i++) {
		GPIO_EnableDI(pConfig->buttons[i].port, pConfig->buttons[i].pin, PULL_UP);
		buttons_[i] = GPIO_GetBitBandIOAddress(&pConfig->buttons[i]);	
	}

	for (int i = 0; i < 4; i++) {
		GPIO_EnableDO(pConfig->patternLeds[i].port, pConfig->patternLeds[i].pin, DRIVE_2MA, PULL_DOWN);
		patternLeds_[i] = GPIO_GetBitBandIOAddress(&pConfig->patternLeds[i]);
	}
	
	for (int i = 0; i < 3; i++) {
		GPIO_EnableDO(pConfig->colorLeds[i].port, pConfig->colorLeds[i].pin, DRIVE_2MA, PULL_DOWN);
		colorLeds_[i] = GPIO_GetBitBandIOAddress(&pConfig->colorLeds[i]);
	}
	
	for (int i = 0; i < 8; i++) {
		GPIO_EnableDI(pConfig->dipSwitches[i].port, pConfig->dipSwitches[i].pin, PULL_DOWN);
		dipSwitches_[i] = GPIO_GetBitBandIOAddress(&pConfig->dipSwitches[i]);
	}
	
	I2C_InitModule(pConfig->i2cModule);
	
	//  Use Standard 100Kbps mode.
	I2C_EnableAsMaster(pConfig->i2cModule, 100000, false);
	
	uint8_t ownAddress = SLAVE_ADDRESS_BASE | (*slavePins_[1] << 1) | *slavePins_[0];
	I2C_EnableAsSlave(pConfig->i2cModule, ownAddress);
	I2C_EnableSlaveDataInterrupt(pConfig->i2cModule, 7, ReceiveCallback, TransmitCallback);
	
	__enable_irq();
	
	return 0;
}


void ReadDIPSwitches(void)
{	
	settings_.slaveTargetId = (*dipSwitches_[0] << 1) | *dipSwitches_[1];
	settings_.color = (*dipSwitches_[2] << 1) | *dipSwitches_[3];
	settings_.pattern = (*dipSwitches_[4] << 3) | (*dipSwitches_[5] << 2) | (*dipSwitches_[6] << 1) | *dipSwitches_[7];
}

void DoWrite(DeviceConfig_t* pConfig, uint8_t command, uint8_t slaveId, uint8_t color, uint8_t pattern)
{
	char data[3];
	data[0] = command;
	data[1] = color;
	data[2] = pattern;

	// For slave address 0, write to all slaves...
	if (slaveId == 0) {

		// This loop writes to all slaves using a repeated start and stops at the first error (e.g. when no slave
		//	with the target address responds).  The MasterWrite function issues a STOP on the bus when a slave
		//	address byte is NAK'd.
		for (int i = 1, error = 0; !error; i++) {
			error = I2C_MasterWrite(pConfig->i2cModule, SLAVE_ADDRESS_BASE + i, data, 3, (i != 1), false); 		
		}

	}
	else {
		// Write the switch data to a single slave.
		I2C_MasterWrite(pConfig->i2cModule, SLAVE_ADDRESS_BASE + slaveId, data, 3, false, true); 		
	}
	
}

void DoRead(DeviceConfig_t* pConfig, uint8_t command, uint8_t slaveId)
{
	char data[2];
	data[0] = command;

	// First, write the 0xA1=Read command to the slave, letting it know a transmit request is coming.
	if (!I2C_MasterWrite(pConfig->i2cModule, SLAVE_ADDRESS_BASE + slaveId, data, 1, false, false)) {
		
		// Next, use a repeated start to read the data from the slave.
		if (!I2C_MasterRead(pConfig->i2cModule, SLAVE_ADDRESS_BASE + slaveId, data, 2, true, true)) {

			// Set the LEDs to the slave's switch settings.
			SetColorLeds(data[0]);
			SetLedPattern(data[1]);
		}
	}
}

void Blink(void)
{
	static int color = 0;
	static int pattern = 0;

	// Cycle through the colors and patterns.
	color = (color + 1) % 3;
	pattern = (pattern + 1) % 16;
	
	SetColorLeds(color + 1);
	SetLedPattern(pattern);
		
}

uint8_t GetButtonState(void)
{
	static uint32_t previous[2] = { 1, 1 };	
	
	// If the current state is LOW then the button is down.
	// If the previous state was HIGH and the current state is LOW, then the button was pressed.
	uint8_t state = 0;
	uint32_t sw1 = *buttons_[0];
	uint32_t sw2 = *buttons_[1];
	
	if (!sw1) {
		state |= SW1;
		if (previous[0]) {
			state |= (SW1 << 2);
		}
	}
	
	if (!sw2) {
		state |= SW2;
		if (previous[1]) {
			state |= (SW2 << 2);
		}
	}

	previous[0] = sw1;
	previous[1] = sw2;
	
	return state;
}


int Run(DeviceConfig_t* pConfig)
{

	InitHardware(pConfig);
	
	for (;;) {

		ReadDIPSwitches();
		
		uint8_t buttons = GetButtonState();
		
		// One button held down while the other pressed...
		if ((IsButtonDown(buttons, SW1) && WasButtonPressed(buttons, SW2)) ||
			  (IsButtonDown(buttons, SW2) && WasButtonPressed(buttons, SW1)) ) {
			
				// Color and pattern parameters do not matter for blinky.
				DoWrite(pConfig, COMMAND_BLINKY, settings_.slaveTargetId, 0, 0);
				
		}
		// SW1 = Write
		else if (WasButtonPressed(buttons, SW1)) {
		
			DoWrite(pConfig, COMMAND_WRITE, settings_.slaveTargetId, settings_.color, settings_.pattern);
		
			// Cancel blinky mode when ever a button is down.
			settings_.blinkyMode = false;
		
		}
		// SW2 = Read
		else if (WasButtonPressed(buttons, SW2)) {
		
			DoRead(pConfig, COMMAND_READ, settings_.slaveTargetId);
		
			// Cancel blinky mode when ever a button is down.
			settings_.blinkyMode = false;
			
		}
				
		if (settings_.blinkyMode) {
			Blink();
		}
		

		SysTick_Wait10ms(10);
			
	}
	
}
