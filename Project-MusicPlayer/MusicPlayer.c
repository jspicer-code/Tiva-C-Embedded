#include "MusicPlayer.h"
#include "DisplayManager.h"
#include "Strings.h"
#include "Random.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include "Song.h"

// Minimum stack size for FreeRTOS tasks.
#define STACK_SIZE 100

// Percussion noise duration is appoximately 2ms.
#define NOISE_DURATION	160000

typedef enum { 
	PLAYER_STOPPED = 0, 
	PLAYER_STOPPING, 
	PLAYER_PLAYING, 
	PLAYER_NAVIGATING, 
	PLAYER_MESSAGED, 
	PLAYER_DOWNLOADING 
} PlayerStates_t;

typedef enum {
	EVENT_NONE = 0,
	EVENT_PLAY,
	EVENT_FINISHED,
	EVENT_NAVIGATE,
	EVENT_MESSAGE,
	EVENT_DOWNLOADED
} PlayerEvents_t;

typedef enum {
	NAVIGATE_NONE = 0,
	NAVIGATE_NEXT,
	NAVIGATE_PREV,
} NavigateDirection_t;

typedef enum { 
	DOWNLOAD_IDLE = 0, 
	DOWNLOAD_RECEIVING, 
	DOWNLOAD_FINISHED 
} DownloadStates_t;

typedef enum {
	BUTTON_NONE = 0,
	BUTTON_PLAY,
	BUTTON_NEXT,
	BUTTON_PREV
} ButtonPressed_t;

struct {
	// There appears to be a 16-bit alignment requirement, so making code 16-bits in size.
	uint16_t	code;
	uint16_t 	size;
	uint8_t		payload[1024];
} message_;

struct { 
	uint8_t playNoise;
	uint32_t noiseTime;
	PWMDef_t pwm;
}	noiseParams_;

struct {
	PlayerStates_t state;
	PlayerEvents_t event;
	ButtonPressed_t buttonPressed;
	NavigateDirection_t navigateDirection;
	int songIndex;
	int songTimeEllapsed;
} playerState_;

SongAlbum_t album_;
Song_t song_;

typedef struct {
	TaskHandle_t taskHandle;
	SongTrack_t* track;
	SemaphoreHandle_t startSemaphore;
	SemaphoreHandle_t stoppedSemaphore;
	volatile uint32_t* led;
	PWMDef_t pwm;
	uint8_t abort;
} TrackParams_t;

static TrackParams_t trackParams_[SONG_MAX_TRACKS];

static SemaphoreHandle_t acceptEventSemaphore_;
static SemaphoreHandle_t dispatchEventSemaphore_;
static SemaphoreHandle_t buttonPressedSemaphore_;
static SemaphoreHandle_t messageSemaphore_;
static SemaphoreHandle_t songMonitorSemaphore_;

static TimerHandle_t songTimer_;

static MusicPlayer_Config_t* pConfig_;

// SystemCoreClock is referenced in FreeRTOSConfig.h.  It will be defined here and assigned
//	the bus clock frequency when the hardware is initialized.
uint32_t SystemCoreClock;

// Catch-all error handler.
static void ErrHandler(void) 
{
		// Spin...
		while (1);
}


static void LoadSong(int index) 
{
	if (album_.header == NULL) {
		Display_ShowFileNotLoadedScreen();
	}
	else if (index >= 0 && index <= album_.header->numSongs) {
	
		uint8_t* address = (uint8_t*)album_.header + album_.songOffsets[index];
		
		SongFileHeader_t* pSongFileHeader = (SongFileHeader_t *)address;
		if (pSongFileHeader->signature != SONG_FILE_SIGNATURE) {
			Display_ShowFileNotLoadedScreen();
		}
		else {
		
			song_.header = pSongFileHeader;
			address += sizeof(SongFileHeader_t);
			
			for (int i = 0; i < song_.header->numTracks; i++) {
				
				SongTrackHeader_t *pTrackHeader = (SongTrackHeader_t *)address;
				song_.tracks[i].notes = (SongNoteEvent_t*)((uint8_t*)song_.header + pTrackHeader->offset);
				
				address += sizeof(SongTrackHeader_t);
			}

			for (int i = 0; i < song_.header->numTracks; i++ ) {		
				trackParams_[i].track = &song_.tracks[i];
			}	
			
			Display_ShowSongScreen(&song_, index + 1, album_.header->numSongs);
			Display_ShowPlayDuration(0, song_.header->duration);
		
			playerState_.songIndex= index;

		}
	}
}


static void LoadAlbum()
{
	album_.header = NULL;
	album_.songOffsets = NULL;
	playerState_.songIndex = -1;
	
	uint8_t* address = (uint8_t*)pConfig_->flashBaseAddress;
	
	SongAlbumHeader_t* pAlbumHeader = (SongAlbumHeader_t *)address;
	if (pAlbumHeader->signature != SONG_ALBUM_SIGNATURE) {
		Display_ShowFileNotLoadedScreen();
	}
	else if (pAlbumHeader->numSongs > 0) {
		
		album_.header = pAlbumHeader;
		album_.songOffsets = (uint32_t*)(address + sizeof(SongAlbumHeader_t));
		
		// Load up the first song.
		LoadSong(0);
	}
}

static void MessageHandler(char c)
{
	static uint16_t bytesReceived = 0;
	
	((uint8_t*)&message_)[bytesReceived++] = (uint8_t)c;
	
	if (bytesReceived >= 4 && (bytesReceived - 4 == message_.size)) {
	
		// This will attempt to wake the MessageTask and continue execution there.
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
		// Give the semaphore and unblock the MessageTask.
		xSemaphoreGiveFromISR(messageSemaphore_, &xHigherPriorityTaskWoken);
		
		// If the MessageTask was successfully woken, then yield execution to it
		//	and go there now (instead of changing context to another task).
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		
		bytesReceived = 0;
				
	}
	
}

static void AckMessage(int percentage)
{
	Display_ShowDownloadingScreen(percentage);
	
	// Acknowledge the message.
	UART_WriteString(pConfig_->uart, "\xCA\xAC");
}

static void MessageTask(void *pvParameters) 
{
	static BaseType_t isIdle = pdTRUE;
	static uint32_t totalExpected = 0;
	static uint32_t totalReceived = 0;
	static int blockCount = 0;
	
	for (;;) {
			
		// Block until a message is received...
		BaseType_t taken = xSemaphoreTake(messageSemaphore_, portMAX_DELAY);
		if (taken == pdFAIL) {
			continue;
		}
			
		if (isIdle) {

			totalReceived = 0;
			totalExpected = *((uint32_t*)&message_.payload);
			blockCount = 0;
	
			isIdle = pdFALSE;		
				
			// Signal to the main task that the first message has arrived.
			xSemaphoreTake(acceptEventSemaphore_, portMAX_DELAY);
			playerState_.event = EVENT_MESSAGE;			
			xSemaphoreGive(dispatchEventSemaphore_);			
			
		}
		else {
			
			int wordCount = (message_.size + (sizeof(uint32_t) - 1)) / sizeof(uint32_t);
			volatile uint32_t* address = pConfig_->flashBaseAddress + (blockCount * FLASH_BLOCK_SIZE_WORDS);
			Flash_Write(address, message_.payload, wordCount);
		
			blockCount++;
			totalReceived += message_.size;
			
			int percentage = ((float)totalReceived / (float)totalExpected) * 100;
			AckMessage(percentage);
			
			// TODO: remove this check for percentage == 100.  This was put in here for 
			//	troubleshooting and wasn't intended to remain. If totalReceived != totalExpected
			//  when percentage >= 100, then an error has occurred somewhere.
			if (percentage == 100 || (totalReceived == totalExpected)) {

				// Show 100% awhile before continuing.
				vTaskDelay(pdMS_TO_TICKS(500));
				
				LoadAlbum();		
		
				isIdle = pdTRUE;
			
				// Signal to the main task that the download is complete.
				xSemaphoreTake(acceptEventSemaphore_, portMAX_DELAY);			
				playerState_.event = EVENT_DOWNLOADED;			
				xSemaphoreGive(dispatchEventSemaphore_);
	
			}			
		}						
	}
}


static void ButtonHandler(PortName_t port, uint32_t pinMap)
{
	
	// Disable interrupts for all buttons.
	GPIO_DisarmInterrupt(&pConfig_->playButton);
	GPIO_DisarmInterrupt(&pConfig_->nextButton);
	GPIO_DisarmInterrupt(&pConfig_->prevButton);
		
	// Record which button was pressed for the button task.
	if (port == pConfig_->playButton.port && pConfig_->playButton.pin & pinMap) {
		playerState_.buttonPressed = BUTTON_PLAY;
	}
	else if (port == pConfig_->nextButton.port && pConfig_->nextButton.pin & pinMap) {
		playerState_.buttonPressed = BUTTON_NEXT;
	}
	else if (port == pConfig_->prevButton.port && pConfig_->prevButton.pin & pinMap) {
		playerState_.buttonPressed = BUTTON_PREV;
	}
	else {
		// Shouldn't be here, but just in case.
		playerState_.buttonPressed = BUTTON_NONE;
	}
		
	// This will attempt a wake the higher priority SwitchTask and continue
	//	execution there.
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	// Give the semaphore and unblock the ButtonTask.
	xSemaphoreGiveFromISR(buttonPressedSemaphore_, &xHigherPriorityTaskWoken);
	
	// If the ButtonTask was successfully woken, then yield execution to it
	//	and go there now (instead of changing context to another task).
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

static void ButtonTask(void *pvParameters) 
{
	TickType_t debounceDelay = pdMS_TO_TICKS(250);
	
	for (;;) {
		
		GPIO_RearmInterrupt(&pConfig_->playButton);
		GPIO_RearmInterrupt(&pConfig_->nextButton);
		GPIO_RearmInterrupt(&pConfig_->prevButton);
		
		// Block until a button is pressed...
		BaseType_t taken = xSemaphoreTake(buttonPressedSemaphore_, portMAX_DELAY);
		if (taken == pdPASS) {
	
			xSemaphoreTake(acceptEventSemaphore_, portMAX_DELAY);
			
			switch (playerState_.buttonPressed) {
				
				case BUTTON_PLAY:
					playerState_.event = EVENT_PLAY;
					playerState_.navigateDirection = NAVIGATE_NONE;
					break;
				
				case BUTTON_NEXT:
					playerState_.event = EVENT_NAVIGATE;
					playerState_.navigateDirection = NAVIGATE_NEXT;
					break;
				
				case BUTTON_PREV:
					playerState_.event = EVENT_NAVIGATE;
					playerState_.navigateDirection = NAVIGATE_PREV;
					break;
				
				case BUTTON_NONE:
					// This shouldn't happen, but just in case...
					continue;
				
			}
			
			xSemaphoreGive(dispatchEventSemaphore_);		

			// Wait a bit to debounce the button.
			vTaskDelay(debounceDelay);			
			
		}
			
	}
	
}

void NoiseTimerCallback(void)
{
	if (noiseParams_.noiseTime < NOISE_DURATION) {
	
		uint32_t r = Random_uint32();
		float f = (float)r / (float)0xFFFFFFFF;

		// Generate a random frequency between 7kHz and 15kHz.
		uint16_t period = 83 + (uint16_t)(f * (float)95);
		
		// Reducing the duty cyle sharpens the sound.
		uint16_t duty = period / 12;
		
		// Playing for less than the period reduces some of the random "tapping" sounds.  
		uint32_t interval = 0.8*period;
		
		PWM_Enable(noiseParams_.pwm, period, duty);	
		Timer_Start(pConfig_->noiseTimer, interval);
		noiseParams_.noiseTime += interval;
	}
	else {
		Timer_Stop(pConfig_->noiseTimer);
		PWM_Disable(noiseParams_.pwm);
		
		// Reseed the random number generator to produce the same frequency sequence for each percussion note.
		Random_Reseed();
	}
	
}

static void PlayNote(const SongNoteEvent_t* note, TrackParams_t* params)
{

	uint8_t channel = 0x0F & note->status;
	
	// A zero note or velocity means silence.
	if (!note || note->key == 0 || note->velocity == 0) {

		if (channel == 9) {
				noiseParams_.pwm = params->pwm;
				noiseParams_.noiseTime = NOISE_DURATION;
				NoiseTimerCallback();
		}
		else {
			PWM_Disable(params->pwm);
		}
		
		// Turn OFF the track LED
		*(params->led) = 0;
	}
	else if (note->key >= SONG_MIN_NOTE && note->key <= SONG_MAX_NOTE) {
	
		if (channel == 9) {
			noiseParams_.pwm = params->pwm;
			noiseParams_.noiseTime = 0;
			NoiseTimerCallback();
		}
		else {	
			// For each note create a 50% duty cycle.
			uint16_t period = Song_NotePwmPeriods[note->key];
			uint16_t duty = period / 2;	
			PWM_Enable(params->pwm, period, duty);
		}
		
		// Turn ON the track LED
		*(params->led) = 1;
	}

}


void TrackTask(void *pvParameters)
{
	TrackParams_t* params = (TrackParams_t*)pvParameters;

	for (;;) {
		
		// Block until its time to play...
		BaseType_t taken = xSemaphoreTake(params->startSemaphore, portMAX_DELAY);
		if (taken == pdPASS) {
		
			TickType_t xLastWakeTime = xTaskGetTickCount();
			
			const SongNoteEvent_t* notes = params->track->notes;
			
			// Play each note.  The notes sequence will end with -1 or when playing 
			//	has been stopped (task aborted).
			for (int noteIndex = 0; notes[noteIndex].deltaTime != -1; noteIndex++) {

				int deltaTime = notes[noteIndex].deltaTime;
				if (deltaTime != 0) {
					
					// Wait until it's time to run the event 
					vTaskDelayUntil(&xLastWakeTime, deltaTime);	
					
					// Stop playing if the track has been aborted.
					if (params->abort) {
						break;
					}
				}
					
				// Mask out the channel bits (lower nibble).
				uint8_t status = 0xF0 & notes[noteIndex].status;
				switch (status) {
					
					case NOTE_ON:			
						PlayNote(&notes[noteIndex], params);
						break;

					case NOTE_OFF:
					default:
						PlayNote(0, params);
						break;
				}

			}
			
			// Turn off the sound.
			PlayNote(0, params);
			
			// Give the Stop semaphore to signal that the track is now stopped.
			xSemaphoreGive(params->stoppedSemaphore);
				
		}
	
	}
	
}


void StartPlaying()
{
	// TODO: Check that there is song loaded.
	
	for (int i = 0; i < song_.header->numTracks; i++ ) {		
		trackParams_[i].abort = 0;
		xSemaphoreGive(trackParams_[i].startSemaphore);
	}	
	
	xSemaphoreGive(songMonitorSemaphore_);
	
	Display_ShowPlayDuration(0, song_.header->duration);
	vTimerSetTimerID( songTimer_, ( void * )0);
	xTimerStart(songTimer_, (TickType_t)0);
}


void StopPlaying()
{
	xTimerStop(songTimer_, (TickType_t)0);

	// TODO: Check that there is song loaded.
	
	for (int i = 0; i < song_.header->numTracks; i++ ) {	
		trackParams_[i].abort = 1;
		xTaskAbortDelay(trackParams_[i].taskHandle);
	}
}


void SongTimerCallback( TimerHandle_t xTimer )
{
	static uint32_t songLength = 0;
	
	// Following the example from the FreeRTOS web stie, the number of times this timer
	//	has expired is saved as the timer's ID.
	uint32_t ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );

	if (ulCount == 0) {
		songLength = song_.header->duration;
	}
	
  // Increment the count, then test to see if the timer has expired
  ulCount++;	
	if (ulCount > songLength) {
	
		Display_ShowPlayDuration(song_.header->duration, song_.header->duration);
		
		StopPlaying();
	
	}
	else {
	
		Display_ShowPlayDuration((int)ulCount, song_.header->duration);
		
		vTimerSetTimerID( songTimer_, ( void * )ulCount);

	}
		
}

void SongMonitorTask(void *pvParameters)
{
	
	for (;;) {
		
		// Block until the monitor semaphore is given...
		BaseType_t taken = xSemaphoreTake(songMonitorSemaphore_, portMAX_DELAY);
		if (taken == pdFAIL) {
			continue;
		}
		
		// Wait for the tracks to finish playing...
		for (int i = 0; i < song_.header->numTracks; i++ ) {	
			xSemaphoreTake(trackParams_[i].stoppedSemaphore, portMAX_DELAY);
		}
			
		xSemaphoreTake(acceptEventSemaphore_, portMAX_DELAY);				
		playerState_.event = EVENT_FINISHED;				
		xSemaphoreGive(dispatchEventSemaphore_);			
		
	}	
}

void NavigateSong(NavigateDirection_t direction)
{
	
	if (album_.header != NULL) {
		
		int index = playerState_.songIndex;
		
		if (direction == NAVIGATE_NEXT) {
			
			index++;
			if (index >= album_.header->numSongs) {
				index = 0;
			}
			
		}
		else if (direction == NAVIGATE_PREV) {
			
			index--;
			if (index < 0) {
				index = album_.header->numSongs - 1;
			}
			
		}
		
		LoadSong(index);
		
	}
	
}


void MainTask(void *pvParameters)
{
	
	LoadAlbum();
	
	for (;;) {
		
		xSemaphoreGive(acceptEventSemaphore_);
		
		// Block until an event is dispatched...
		BaseType_t taken = xSemaphoreTake(dispatchEventSemaphore_, portMAX_DELAY);
		if (taken == pdFAIL) {
			continue;
		}
		
		switch (playerState_.state) {
		
			case PLAYER_STOPPED:
				
				switch (playerState_.event) {
					case EVENT_MESSAGE:
						AckMessage(0);
						playerState_.state = PLAYER_DOWNLOADING;
						break;
					case EVENT_NAVIGATE:
						NavigateSong(playerState_.navigateDirection);
						break;
					case EVENT_PLAY:
						StartPlaying();
						playerState_.state = PLAYER_PLAYING;
						break;
					default:
						break;
				}		
				break;
			
			case PLAYER_STOPPING:
				switch (playerState_.event) {
					case EVENT_FINISHED:
						playerState_.state = PLAYER_STOPPED;
						break;
					case EVENT_MESSAGE:
						playerState_.state = PLAYER_MESSAGED;
						break;
					default:
						break;
				}				
				break;
			
			case PLAYER_PLAYING:
				switch (playerState_.event) {
					case EVENT_FINISHED:
						NavigateSong(NAVIGATE_NEXT);					
						StartPlaying();
						break;					
					case EVENT_MESSAGE:
						StopPlaying();
						playerState_.state = PLAYER_MESSAGED;					
						break;
					case EVENT_NAVIGATE:
						StopPlaying();
						playerState_.state = PLAYER_NAVIGATING;
						break;
					case EVENT_PLAY:
						StopPlaying();
						playerState_.state = PLAYER_STOPPING;
						break;
					default:
						break;
				}			
				break;
			
			case PLAYER_NAVIGATING:
				
				switch (playerState_.event) {
					case EVENT_FINISHED:						
						NavigateSong(playerState_.navigateDirection);					
						StartPlaying();
						playerState_.state = PLAYER_PLAYING;
						break;
					case EVENT_MESSAGE:
						StopPlaying();
						playerState_.state = PLAYER_MESSAGED;
						break;
				}
				break;
			
			case PLAYER_MESSAGED:
				switch (playerState_.event) {
					case EVENT_FINISHED:
						AckMessage(0);						
						playerState_.state = PLAYER_DOWNLOADING;
						break;
				}
				break;
			
			case PLAYER_DOWNLOADING:
				switch (playerState_.event) {
					case EVENT_DOWNLOADED:
						playerState_.state = PLAYER_STOPPED;
						break;
				}
				break;
				
		}
		
	}
}

// Initialize the hardware and peripherals...
static int InitHardware(MusicPlayer_Config_t* pConfig)
{
	__disable_irq();
	
	PLL_Init80MHz();

	// Must store the frequency in SystemCoreClock for FreeRTOS to use.
	SystemCoreClock = PLL_BusClockFreq;

	// Enable button inputs.
	GPIO_EnableDI(pConfig->playButton.port, pConfig->playButton.pin, PULL_UP);
	GPIO_EnableDI(pConfig->nextButton.port, pConfig->nextButton.pin, PULL_UP);
	GPIO_EnableDI(pConfig->prevButton.port, pConfig->prevButton.pin, PULL_UP);
	
	// Enable button interrupts
	GPIO_EnableInterrupt(&pConfig->playButton, 7, INT_TRIGGER_FALLING_EDGE, ButtonHandler);
	GPIO_EnableInterrupt(&pConfig->nextButton, 7, INT_TRIGGER_FALLING_EDGE, ButtonHandler);
	GPIO_EnableInterrupt(&pConfig->prevButton, 7, INT_TRIGGER_FALLING_EDGE, ButtonHandler);
		
	// Initialze PWM channels and GPIO for the track LEDs.
	for (int i = 0; i < 4; i++) {

		PWM_Initialize(pConfig->trackPWM[i]);
		trackParams_[i].pwm = pConfig->trackPWM[i];

		GPIO_EnableDO(pConfig->trackLED[i].port, pConfig->trackLED[i].pin, DRIVE_2MA, PULL_DOWN);
		trackParams_[i].led = GPIO_GetBitBandIOAddress(&pConfig->trackLED[i]);		
	}

	// Slow down the PWM clock frequency in order to play notes
	//	in the neighborhood of middle-C.
	PWM_SetClockDivisor(64);
	
	UART_Enable(pConfig->uart, 115200);
	UART_EnableRxInterrupt(pConfig->uart, 6, MessageHandler);
	
	Flash_Enable();

	if (Display_Init(&pConfig->lcd)) {
		return -1;
	}
	
	Timer_Init(pConfig->noiseTimer, TIMER_ONESHOT, 0, NoiseTimerCallback);
	
	__enable_irq();
	
	return 0;
	
}

int InitTasks()
{
	
	int success = -1;
	
	do {
	
		acceptEventSemaphore_ = xSemaphoreCreateBinary();
		if (!acceptEventSemaphore_) {
			break;
		}
	
		dispatchEventSemaphore_ = xSemaphoreCreateBinary();
		if (!dispatchEventSemaphore_) {
			break;
		}
		
		buttonPressedSemaphore_ = xSemaphoreCreateBinary();
		if (!buttonPressedSemaphore_) {
			break;
		}
		
		messageSemaphore_ = xSemaphoreCreateBinary();
		if (!messageSemaphore_) {
			break;
		}
	
		songMonitorSemaphore_ = xSemaphoreCreateBinary();
		if (!songMonitorSemaphore_) {
			break;
		}

		int trackSemCount = 0;
		for (int i = 0; i < SONG_MAX_TRACKS; i++ ) {		
			trackParams_[i].startSemaphore = xSemaphoreCreateBinary();
			if (!trackParams_[i].startSemaphore) {
				break;
			}
			trackParams_[i].stoppedSemaphore = xSemaphoreCreateBinary();
			if (!trackParams_[i].stoppedSemaphore) {
				break;
			}
			trackSemCount++;
		}
		
		if (trackSemCount != SONG_MAX_TRACKS) {
			break;
		}
		
		songTimer_ = xTimerCreate("SongTimer", configTICK_RATE_HZ, pdTRUE, (void*)0, SongTimerCallback);
		if (!songTimer_) {
			break;
		}
		
		// The button task will have a higher priority than the main task.
		xTaskCreate(ButtonTask, "Button", STACK_SIZE, NULL, 3, NULL);
	
		// The message task will have a higher priority than the main task.
		xTaskCreate(MessageTask, "Message", STACK_SIZE, NULL, 3, NULL);
	
		// The monitor task will have a higher priority than the main task.
		xTaskCreate(SongMonitorTask, "SongMonitor", STACK_SIZE, NULL, 3, NULL);
						
		// The main task will have a higher priority than the track tasks.
		xTaskCreate(MainTask, "Main", 2 * STACK_SIZE, NULL, 2, NULL);
		
		// Create the track tasks at the lowest priority.
		for (int i = 0; i < SONG_MAX_TRACKS; i++ ) {		
			xTaskCreate(TrackTask, "Track", 2 * STACK_SIZE, &trackParams_[i], 1, &trackParams_[i].taskHandle);
		}	
		
		success = 0;
	
	}	while (pdFALSE);
	
	
	return success;
	
}


void MusicPlayer_Run(MusicPlayer_Config_t* pConfig)
{
	
	// Save the pointer to the configuration.
	pConfig_ = pConfig;
	
	if (InitHardware(pConfig)) {
		ErrHandler();
	}
	
	if (InitTasks()) {
		ErrHandler();
	}

	// Startup of the FreeRTOS scheduler.  The program should block here.  
	vTaskStartScheduler();
	
	// The following line should never be reached.  Failure to allocate enough
	//	memory from the heap would be one reason.
	for (;;);

}


