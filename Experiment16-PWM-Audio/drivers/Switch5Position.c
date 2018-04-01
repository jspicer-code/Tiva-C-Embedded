
#include "Switch5Position.h"


static volatile uint32_t*		centerAddr_;
static volatile uint32_t*		upAddr_;
static volatile uint32_t*		downAddr_;
static volatile uint32_t*		leftAddr_;
static volatile uint32_t*		rightAddr_;

static uint8_t current_;
static uint8_t previous_;

int Switch5_Initialize(const Switch5Config_t* config)
{
	
	GPIO_EnableDI(config->centerPin.port, config->centerPin.pin, PULL_NONE);
	centerAddr_ = GPIO_GetBitBandIOAddress(&config->centerPin);
	
	GPIO_EnableDI(config->upPin.port, config->upPin.pin, PULL_NONE);
	upAddr_ = GPIO_GetBitBandIOAddress(&config->upPin);

	GPIO_EnableDI(config->downPin.port, config->downPin.pin, PULL_NONE);
	downAddr_ = GPIO_GetBitBandIOAddress(&config->downPin);

	GPIO_EnableDI(config->leftPin.port, config->leftPin.pin, PULL_NONE);
	leftAddr_ = GPIO_GetBitBandIOAddress(&config->leftPin);
	
	GPIO_EnableDI(config->rightPin.port, config->rightPin.pin, PULL_NONE);
	rightAddr_ = GPIO_GetBitBandIOAddress(&config->rightPin);
	
	return 0;
	
}

uint32_t Switch5_Poll(void)
{	
	previous_ = current_;
	
	current_ = 0;
	current_ |= !(*centerAddr_);
	current_ |= !(*upAddr_) << 1;
	current_ |= !(*downAddr_) << 2;
	current_ |= !(*leftAddr_) << 3;
	current_ |= !(*rightAddr_) << 4;

	uint8_t closed = current_ & ~previous_;
	uint8_t opened = ~current_ & previous_;
	uint32_t state = (opened << 16) | (closed << 8) | current_;
	
	return state;
}
