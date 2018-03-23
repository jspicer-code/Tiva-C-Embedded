/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
 

/******************************************************************************
 *  MODIFICATIONS:
 *  03/19/2018 - Modified to run on the EK-TM4C123GXL development board. jspicer
 */
 
/******************************************************************************
 *
 * main creates one queue, one software timer, and two tasks.  It then
 * starts the scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
 * the value 100 to the queue every 200 milliseconds (please read the notes
 * above regarding the accuracy of timing under Windows).
 *
 * The Queue Send Software Timer:
 * The timer is a one-shot timer that is reset by a SW1 press.  The timer's
 * period is set to two seconds - if the timer expires then its callback
 * function writes the value 200 to the queue.  The callback function is
 * implemented by prvQueueSendTimerCallback() within this file.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
 * When data is received, the task checks the value of the data, then blinks
 * the green LED to indicate that the data came from the queue send task and
 * the red LED to indicate that it came from the queue send software timer.
 *
 * Expected Behaviour:
 * - The queue send task writes to the queue every 200ms, so every 200ms the
 *   queue receive task will blink the green LED indicating that data was received
 *   on the queue from the queue send task.
 * - The queue send software timer has a period of two seconds, and is reset
 *   each time SW1 is pressed.  So if two seconds expire without SW1 being
 *   pressed then the queue receive task will blink the red LED indicating that
 *   data was received on the queue from the queue send software timer.
 *
 */

/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>

/* Hardware includes */
#include "HAL/HAL.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The times are converted from
milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 200UL )
#define mainTIMER_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 2000UL )

/* These are ON durations for the blinking LEDS */
#define mainTASK_BLINK_DELAY_MS					pdMS_TO_TICKS( 20UL )
#define mainTIMER_BLINK_DELAY_MS				pdMS_TO_TICKS( 200UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH					( 2 )

/* The values sent to the queue receive task from the queue send task and the
queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK			( 100UL )
#define mainVALUE_SENT_FROM_TIMER			( 200UL )

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/*
 * The callback function executed when the software timer expires.
 */
static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;

/*-----------------------------------------------------------*/


/* This variable is referenced in FreeRTOSConfig.h and will be used to define configCPU_CLOCK_HZ. */
/* The default value is 16MHz, but this will be overwritten with the actual bus frequency used		*/
/* when the PLL is configured. 																																		*/
uint32_t SystemCoreClock = 16000000;

/* These are the bit-band addresses for the onboard LEDs  */
volatile uint32_t* redLED_;
volatile uint32_t* greenLED_;

void BlinkLED(volatile uint32_t* led, const TickType_t onDuration)
{
	*led = 1;
	vTaskDelay(onDuration);
	*led = 0;
}

void SwitchHandler(uint32_t pinMap)
{
	
	// Disable interrupts for SW1
	GPIO_DisarmInterrupt(&PINDEF(PORTF, (PinName_t)(PIN4)));
	
	// This will attempt a wake a higher priority task and continue execution there.
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Reset the timer when SW1 has been pressed.  The timer will write
	// mainVALUE_SENT_FROM_TIMER to the queue when it expires.
	xTimerResetFromISR( xTimer, &xHigherPriorityTaskWoken );
	
	// If a higher priority was successfully woken, then yield execution to it
	//	and go there now (instead of changing context to another task).
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	
}


void InitHardware(void)
{
	
	// Enable the PLL and initialize the bus frequency to 80MHz.
	PLL_Init80MHz();

	// Store the frequency in SystemCoreClock for FreeRTOS to use.
	SystemCoreClock = PLL_GetBusClockFreq();

	// Enable the onboard LEDs.
	GPIO_EnableDO(PORTF, PIN1 | PIN3, DRIVE_2MA, PULL_DOWN);
	
	// Get the bit band addresses of the LEDs.
	redLED_ = GPIO_GetBitBandIOAddress(&PINDEF(PORTF, PIN1));
	greenLED_ = GPIO_GetBitBandIOAddress(&PINDEF(PORTF, PIN3));
	
	// Enable digital intput for SW1.
	GPIO_EnableDI(PORTF, PIN4, PULL_UP);
	
	// Enable interrupts for SW1, lowest priority.
	GPIO_EnableInterrupt(&PINDEF(PORTF, PIN4), 7, INT_TRIGGER_FALLING_EDGE, SwitchHandler);
	
	// Turn off the LEDs.
	*greenLED_ = 0;
	*redLED_ = 0;

}



/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
int main( void )
{
	const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;
	
	InitHardware();
	
	/* Create the queue. */
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	if( xQueue != NULL )
	{
		/* Start the two tasks as described in the comments at the top of this file. */
		xTaskCreate( prvQueueReceiveTask,			/* The function that implements the task. */
					"Rx", 													/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
					NULL, 													/* The parameter passed to the task - not used in this simple case. */
					mainQUEUE_RECEIVE_TASK_PRIORITY,/* The priority assigned to the task. */
					NULL );													/* The task handle is not required, so NULL is passed. */

		xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		/* Create the software timer, but don't start it yet. */
		xTimer = xTimerCreate( "Timer",					/* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
								xTimerPeriod,								/* The period of the software timer in ticks. */
								pdFALSE,										/* xAutoReload is set to pdFALSE, so this is a one shot timer. */
								NULL,												/* The timer's ID is not used. */
								prvQueueSendTimerCallback );/* The function executed when the timer expires. */

		/* Start the tasks and timer running. */
		vTaskStartScheduler();
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
	TickType_t xNextWakeTime;
	const TickType_t xBlockTime = mainTASK_SEND_FREQUENCY_MS;
	const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK;

	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again.
		The block time is specified in ticks, pdMS_TO_TICKS() was used to
		convert a time specified in milliseconds into a time specified in ticks.
		While in the Blocked state this task will not consume any CPU time. */
		vTaskDelayUntil( &xNextWakeTime, xBlockTime );

		/* Send to the queue - causing the queue receive task to unblock and
		write to the console.  0 is used as the block time so the send operation
		will not block - it shouldn't need to block as the queue should always
		have at least one space at this point in the code */
		xQueueSend( xQueue, &ulValueToSend, 0U );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle )
{
	const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;

	/* This is the software timer callback function.  The software timer has a
	period of two seconds and is reset each time a SW1 is pressed.  This
	callback function will execute if the timer expires, which will only happen
	if a key is not pressed for two seconds. */

	/* Avoid compiler warnings resulting from the unused parameter. */
	( void ) xTimerHandle;

	/* Send to the queue - causing the queue receive task to unblock and
	write out a message.  This function is called from the timer/daemon task, so
	must not block.  Hence the block time is set to 0. */
	xQueueSend( xQueue, &ulValueToSend, 0U );
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
	uint32_t ulReceivedValue;

	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h.  It will not use any CPU time while it is in the
		Blocked state. */
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

		/*  To get here something must have been received from the queue */
		if( ulReceivedValue == mainVALUE_SENT_FROM_TASK )
		{
			// Blink green LED
			BlinkLED(greenLED_, mainTASK_BLINK_DELAY_MS	);
			
		}
		else if( ulReceivedValue == mainVALUE_SENT_FROM_TIMER )
		{
			// Blink red LED
			BlinkLED(redLED_, mainTIMER_BLINK_DELAY_MS	);
			
			// Rearm interrupts for SW1.
			GPIO_RearmInterrupt(&PINDEF(PORTF, (PinName_t)(PIN4)));
		}
		
	}
}
/*-----------------------------------------------------------*/
