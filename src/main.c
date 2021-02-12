#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"
#include "TUM_Print.h"

#include "AsyncIO.h"

#include "tetrimino.h"
#include "grid.h"
#include "tetris.h"
#include "frontend_adapter.h"
#include "menu.h"

// =============================================================================
// Tasks Handles
// =============================================================================
static TaskHandle_t StateMachine = NULL;
static TaskHandle_t BufferSwap = NULL;
TaskHandle_t GameplayTask = NULL;
TaskHandle_t MainMenuTask = NULL;
TaskHandle_t PauseTask = NULL;

// =============================================================================
// Task Functions
// =============================================================================
extern void vMainMenuTask();
extern void vPauseTask();
// =============================================================================
// Queues
// =============================================================================
QueueHandle_t StateQueue = NULL;

// =============================================================================
// Semaphores
// =============================================================================
SemaphoreHandle_t DrawSignal = NULL;
SemaphoreHandle_t ScreenLock = NULL;
SemaphoreHandle_t GameEngineLock = NULL; //Only one task holding the Game Engine Lock can modify it

// =============================================================================
// Structures
// =============================================================================
typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    unsigned char lastButtonState[SDL_NUM_SCANCODES];
    TickType_t  last_change[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;


// =============================================================================
// Handles
// =============================================================================
static buttons_buffer_t buttons = { 0 };
image_handle_t gameplay_background = NULL;
image_handle_t mainmenu_background = NULL;
image_handle_t pause_background = NULL;

//Adapter between TUMDraw and the game engine
FrontendAdapter_t mFrontendAdapter = {0};
// Basic block in the game is called a tetrimino
Tetrimino_t pmTetriminos = {0};
// The grid contains tetrimintos
Grid_t mGrid = {0};
// The tetris game engine
Tetris_t mGame = {0};

// =============================================================================
// Defintions
// =============================================================================
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define STATE_QUEUE_LENGTH 1


#define STATE_COUNT 3

#define STATE_ONE 0
#define STATE_TWO 1
#define STATE_THREE 2

#define NEXT_TASK 0
#define PREV_TASK 1

#define STARTING_STATE STATE_ONE

#define STATE_DEBOUNCE_DELAY 300

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define STATIC_STACK_SIZE 0


const unsigned char next_state_signal = NEXT_TASK;
const unsigned char prev_state_signal = PREV_TASK;








/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static – otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task’s
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task’s stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*———————————————————–*/

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}



void checkDraw(unsigned char status, const char *msg)
{
    if (status) {
        if (msg)
            fprints(stderr, "[ERROR] %s, %s\n", msg,
                    tumGetErrorMessage());
        else {
            fprints(stderr, "[ERROR] %s\n", tumGetErrorMessage());
        }
    }   
}
/*  
 *   The Task that refreshes the screen at a constant frame rate
 */
void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    tumDrawBindThread(); // Setup Rendering handle with correct GL context

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
            tumDrawUpdateScreen();
            tumEventFetchEvents(FETCH_EVENT_BLOCK);
            xSemaphoreGive(ScreenLock);
            xSemaphoreGive(DrawSignal);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(frameratePeriod));
        }
    }
}



#define FPS_AVERAGE_COUNT 50

void vDrawFPS(void)
{
    static unsigned int periods[FPS_AVERAGE_COUNT] = { 0 };
    static unsigned int periods_total = 0;
    static unsigned int index = 0;
    static unsigned int average_count = 0;
    static TickType_t xLastWakeTime = 0, prevWakeTime = 0;
    static char str[10] = { 0 };
    static int text_width;
    int fps = 0;

    if (average_count < FPS_AVERAGE_COUNT) {
        average_count++;
    }
    else {
        periods_total -= periods[index];
    }

    xLastWakeTime = xTaskGetTickCount();

    if (prevWakeTime != xLastWakeTime) {
        periods[index] =
            configTICK_RATE_HZ / (xLastWakeTime - prevWakeTime);
        prevWakeTime = xLastWakeTime;
    }
    else {
        periods[index] = 0;
    }

    periods_total += periods[index];

    if (index == (FPS_AVERAGE_COUNT - 1)) {
        index = 0;
    }
    else {
        index++;
    }

    fps = periods_total / average_count;

  //  tumFontSelectFontFromName(FPS_FONT);

    sprintf(str, "FPS: %2d", fps);

    if (!tumGetTextSize((char *)str, &text_width, NULL))
        checkDraw(tumDrawText(str, SCREEN_WIDTH - text_width - 20,
                              SCREEN_HEIGHT - DEFAULT_FONT_SIZE * 2,
                              Skyblue),
                  __FUNCTION__);

}

/*
 * Changes the state, either forwards of backwards
 */
void changeState(volatile unsigned char *state, unsigned char forwards)
{
    switch (forwards) {
        case NEXT_TASK:
            if (*state == STATE_COUNT - 1) {
                *state = 0;
            }
            else {
                (*state)++;
            }
            break;
        case PREV_TASK:
            if (*state == 0) {
                *state = STATE_COUNT - 1;
            }
            else {
                (*state)--;
            }
            break;
        default:
            break;
    }
}

/*
 * Example basic state machine with sequential states
 */
void basicSequentialStateMachine(void *pvParameters)
{
    unsigned char current_state = STARTING_STATE; // Default state
    unsigned char state_changed =
        1; // Only re-evaluate state if it has changed
    unsigned char input = 0;

    const int state_change_period = STATE_DEBOUNCE_DELAY;

    TickType_t last_change = xTaskGetTickCount();

    while (1) {
        if (state_changed) {
            goto initial_state;
        }

        // Handle state machine input
        if (StateQueue)
            if (xQueueReceive(StateQueue, &input, portMAX_DELAY) ==
                pdTRUE)
                if (xTaskGetTickCount() - last_change >
                    state_change_period) {
                    changeState(&current_state, input);
                    state_changed = 1;
                    last_change = xTaskGetTickCount();
                }

initial_state:
        // Handle current state
        if (state_changed) {
            switch (current_state) {
                case STATE_ONE:         //Main Menu
                    if (GameplayTask) {
                        vTaskSuspend(GameplayTask);
                    }
                    if (PauseTask) {
                        vTaskSuspend(PauseTask);
                    }
                    if (MainMenuTask) {
                        vTaskResume(MainMenuTask);
                    }
                    break;
                case STATE_TWO:           //Gameplay
                    if (MainMenuTask) {
                        vTaskSuspend(MainMenuTask);
                    }
                    if (PauseTask) {
                        vTaskSuspend(PauseTask);
                    }
                    if (GameplayTask) {
                        vTaskResume(GameplayTask);
                    }
                    break;
                case STATE_THREE:       //Pause Screen
                    if (GameplayTask) {
                        vTaskSuspend(GameplayTask);
                    }
                    if (MainMenuTask) {
                        vTaskSuspend(MainMenuTask);
                    }
                    if (PauseTask) {
                        vTaskResume(PauseTask);
                    }
                    break;
                default:
                    break;
            }
            state_changed = 0;
        }
    }
}


void xGetButtonInput(void)      //Takes a copy of the state of the buttons
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}


#define GAME_SCREEN_HEIGHT (SCREEN_HEIGHT-37)

int checkButton(int buttonIndex)
{
    int buttonState = 0;
    int ret = 0;
    const int debounce_delay = pdMS_TO_TICKS(20);
    
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE)
    {
        buttonState = buttons.buttons[buttonIndex];
    }
    

    // compare the buttonState to its previous state
    if (buttonState != buttons.lastButtonState[buttonIndex]) {
        // if the state has changed, debounce and incremenet
        if (xTaskGetTickCount() - buttons.last_change[buttonIndex] > debounce_delay) {
            if (buttonState ==1 ) {
                // if the current state is HIGH then the button went from off to on:
                ret = 1;
            }
            buttons.last_change[buttonIndex] = xTaskGetTickCount();
        }
    }
    // save the current state as the last state, for next time through the loop
    buttons.lastButtonState[buttonIndex] = buttonState;
    xSemaphoreGive(buttons.lock);

    return ret;
}

 void vGameplayTask()
 {

    char buffer[20];    //for printing to the screen

	// Current Time
	TickType_t mTime1 = xTaskGetTickCount();

     while(1){
        if (DrawSignal)
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
                pdTRUE){
                if (xSemaphoreTake(GameEngineLock, portMAX_DELAY) == pdTRUE)
                {
                    tumEventFetchEvents(FETCH_EVENT_BLOCK |
                                        FETCH_EVENT_NO_GL_CHECK);

                    xSemaphoreTake(ScreenLock, portMAX_DELAY);
                        mFrontendAdapter.ClearScreen();                      // Clear screen
                        mGame.DrawScene(&mGame);                // Draw Game   
                        vDrawFPS();                             // Draw FPS in lower right corner
                        sprintf(buffer, "%d", mGrid.mScore);
                        tumDrawText(buffer,70,110, White );
                        sprintf(buffer, "%d", mGrid.mLevel);
                        tumDrawText(buffer,70,240, White );
                        sprintf(buffer, "%d", mGrid.mRemovedLineCount);
                        tumDrawText(buffer,70,360, White );
                    xSemaphoreGive(ScreenLock);

                    xGetButtonInput(); // Update global input

                    if (checkButton(SDL_SCANCODE_RIGHT)){
                        if (mGrid.IsPossibleMovement (&mGrid ,mGame.mPosX + 1, mGame.mPosY, mGame.mTetrimino, mGame.mRotation))
                        {
                            mGame.mPosX++;
                            mGame.calculateShadowPiece(&mGame);
                        }

                    }
                    if (checkButton(SDL_SCANCODE_LEFT)){
                        if (mGrid.IsPossibleMovement (&mGrid ,mGame.mPosX - 1, mGame.mPosY, mGame.mTetrimino, mGame.mRotation))
                        {
                            mGame.mPosX--;	
                            mGame.calculateShadowPiece(&mGame);
                        }

                    }
                    if (checkButton(SDL_SCANCODE_DOWN)){    //Soft Drop
                        if (mGrid.IsPossibleMovement (&mGrid ,mGame.mPosX, mGame.mPosY + 1, mGame.mTetrimino, mGame.mRotation))
                        {
                        mGame.mPosY++;	      
                        mGame.calculateShadowPiece(&mGame);
                        }

                    }
                    if (checkButton(SDL_SCANCODE_SPACE)){   //Rotation
                        if (mGrid.IsPossibleMovement(
                                &mGrid ,mGame.mPosX, mGame.mPosY, mGame.mTetrimino,
                                (mGame.mRotation + 1) % 4))
                            {
                            mGame.mRotation = (mGame.mRotation + 1) % 4;
                            mGame.calculateShadowPiece(&mGame);
                            }

                    }
                    if (checkButton(SDL_SCANCODE_UP)){      //Hard drop
                        mGame.HardDrop(&mGame);
                        if (mGrid.IsGameOver(&mGrid)) {
                            if(mGrid.mScore > mGrid.mHighScore) 
                            {
                                mGrid.mHighScore = mGrid.mScore;
                            }
                            xSemaphoreGive(GameEngineLock);
                            xQueueSend(StateQueue, &prev_state_signal, 0);
                        }
                    }
                    
                    TickType_t mTime2 = xTaskGetTickCount();

                    if ((mTime2 - mTime1) > pdMS_TO_TICKS(mGrid.waitTimeinMS(&mGrid)))
                    {
                        //Caluclate Shadow Piece
                        mGame.mShadowPosY = -3;
                        while (mGrid.IsPossibleMovement(&mGrid, mGame.mPosX, mGame.mShadowPosY, mGame.mTetrimino, mGame.mRotation))
                        {
                            mGame.mShadowPosY++;
                        }
                        //Caluclate Shadow Piece

                        
                        
                        if (mGrid.IsPossibleMovement (&mGrid,mGame.mPosX, mGame.mPosY + 1, mGame.mTetrimino, mGame.mRotation))
                        {
                            mGame.mPosY++;
                        }
                        else
                        {
                            mGrid.MergeTetrimino (&mGrid, mGame.mPosX, mGame.mPosY, 
                                                mGame.mTetrimino, mGame.mRotation);

                            mGrid.RemoveFullLines (&mGrid);

                            mGrid.updateLevel(&mGrid);

                            if (mGrid.IsGameOver(&mGrid))
                            {
                                if(mGrid.mScore > mGrid.mHighScore) //Update High Score
                                {
                                    mGrid.mHighScore = mGrid.mScore;
                                }
                                if (StateQueue) {
                                    xSemaphoreGive(GameEngineLock);
                                    xQueueSend(StateQueue, &prev_state_signal, 0);
                                }
                            }

                            mGame.CreateNewPiece(&mGame);
                        }

                        mTime1 = xTaskGetTickCount();
                    }
                    if(isPausePressed())
                    {
                        xSemaphoreGive(GameEngineLock);
                        xQueueSend(StateQueue, &next_state_signal, 0);
                    }
                xSemaphoreGive(GameEngineLock);
                }
            }
     }
 }

#define SAMPLE_FOLDER "/../resources/waveforms/"

#define GEN_FULL_SAMPLE_PATH(SAMPLE) SAMPLE_FOLDER #SAMPLE ".wav",

#define PRINT_TASK_ERROR(task) PRINT_ERROR("Failed to print task ##task");

int main(int argc, char *argv[])
{
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);

    printf("Initializing: ");

    if (tumDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }

    if (tumEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    if (tumSoundInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }

    gameplay_background = tumDrawLoadImage("gameplay_background.png");
    mainmenu_background = tumDrawLoadImage("mainmenu_background.png");
    pause_background =    tumDrawLoadImage("pause_background.png");



    buttons.lock = xSemaphoreCreateMutex(); // Creates a Mutex and assigns it to the lock in the buttons structure
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_buttons_lock;
    }


    DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
    if (!DrawSignal) {
        PRINT_ERROR("Failed to create draw signal");
        goto err_draw_signal;
    }
    ScreenLock = xSemaphoreCreateMutex();
    if (!ScreenLock) {
        PRINT_ERROR("Failed to create screen lock");
        goto err_screen_lock;
    }


    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES-1,
                    BufferSwap) != pdPASS) {
        PRINT_TASK_ERROR("BufferSwapTask");
        goto err_bufferswap;
    }


    StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));
    if (!StateQueue) {
        PRINT_ERROR("Could not open state queue");
        goto err_state_queue;
    }

    if (xTaskCreate(basicSequentialStateMachine, "StateMachine",
                    mainGENERIC_STACK_SIZE * 2, NULL,
                    configMAX_PRIORITIES - 1, StateMachine) != pdPASS) {
        PRINT_TASK_ERROR("StateMachine");
        goto err_statemachine;
    }


    if (xTaskCreate(vGameplayTask, "Gameplay Task", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &GameplayTask) != pdPASS) {
        goto err_demotask;
    }

    if (xTaskCreate(vMainMenuTask, "Main Menu Task", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &MainMenuTask) != pdPASS) {
        goto err_menutask;
    }

    if (xTaskCreate(vPauseTask, "Pause Menu Task", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &PauseTask) != pdPASS) {
        goto err_pausetask;
    }


    GameEngineLock = xSemaphoreCreateMutex(); 
    if (!GameEngineLock) {
        PRINT_ERROR("Failed to create Game Engine Lock lock");
        goto err_engine_lock;
    }    

    char *waveFileNames = "/../resources/waveforms/tetris.wav";

    char *fullWaveFileNames = { 0 };

    size_t bin_dir_len = strlen(bin_folder_path);

    fullWaveFileNames = calloc(
	    1, sizeof(char) * (strlen(waveFileNames) + bin_dir_len + 1));


    strcpy(fullWaveFileNames, bin_folder_path);
    strcat(fullWaveFileNames, waveFileNames);

    tumSoundLoadUserSample(fullWaveFileNames);
    tumSoundPlayUserSample(fullWaveFileNames);

    frontendAdapter_init(&mFrontendAdapter);
    tetrimino_init(&pmTetriminos);
    grid_init(&mGrid, &pmTetriminos, GAME_SCREEN_HEIGHT);
    tetris_init(&mGame, &mGrid, &pmTetriminos, &mFrontendAdapter, GAME_SCREEN_HEIGHT);
    
    vTaskSuspend(MainMenuTask);
    vTaskSuspend(GameplayTask);
    vTaskSuspend(PauseTask);


    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_musictask:
    vSemaphoreDelete(GameEngineLock);
err_engine_lock:
    vTaskDelete(PauseTask);
err_pausetask:
    vTaskDelete(MainMenuTask);
err_menutask:
    vTaskDelete(GameplayTask);
err_demotask:
    vTaskDelete(StateMachine);
err_statemachine:
    vQueueDelete(StateQueue);
err_state_queue:
    vTaskDelete(vSwapBuffers);
err_bufferswap:
    vSemaphoreDelete(DrawSignal);
err_screen_lock:
    vSemaphoreDelete(DrawSignal);
err_draw_signal:
    vSemaphoreDelete(buttons.lock);
err_buttons_lock:
    tumSoundExit();
err_init_audio:
    tumEventExit();
err_init_events:
    tumDrawExit();
err_init_drawing:
    return EXIT_FAILURE;
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
