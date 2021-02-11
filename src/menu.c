#include "menu.h"
/* Fucntions for managening user input from menus*/

enum MainMenuButton {NONE, START, LEVEL_LEFT, LEVEL_RIGHT, MODE_RIGHT, MODE_LEFT};

extern QueueHandle_t StateQueue;

extern image_handle_t gameplay_background;
extern image_handle_t mainmenu_background;
extern image_handle_t pause_background;
extern image_handle_t gameover_background;

extern const unsigned char next_state_signal;
extern const unsigned char prev_state_signal;

extern SemaphoreHandle_t DrawSignal;
extern SemaphoreHandle_t ScreenLock; 
extern SemaphoreHandle_t GameEngineLock;

extern Tetris_t mGame; 


bool isPausePressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 525 && x <= 580)
    {
        if(y >= 375 && y <= 430)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}

static bool isStartPressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 245 && x <= 400)
    {
        if(y >= 290 && y <= 320)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}

static bool isLevelLeftPressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 245 && x <= 270)
    {
        if(y >= 335 && y <= 365)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}


static bool isLevelRightPressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 320 && x <= 345)
    {
        if(y >= 335 && y <= 365)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}

static bool isModeLeftPressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 245 && x <= 270)
    {
        if(y >= 335 && y <= 365)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}


static bool isModeRightPressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 320 && x <= 345)
    {
        if(y >= 380 && y <= 405)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}


enum MainMenuButton buttonPress(TickType_t *lastButtonPress)
{
    if (xTaskGetTickCount() - *lastButtonPress  > 100)  //Basic Debouncing of Mouse Key
    {
        if(isStartPressed()) {*lastButtonPress = xTaskGetTickCount(); return START;}
        if(isLevelLeftPressed()){*lastButtonPress = xTaskGetTickCount();  return LEVEL_LEFT;}
        if(isLevelRightPressed()){*lastButtonPress = xTaskGetTickCount();  return LEVEL_RIGHT;}
        if(isModeLeftPressed()){*lastButtonPress = xTaskGetTickCount();  return MODE_RIGHT;}
        if(isModeRightPressed()){*lastButtonPress = xTaskGetTickCount();  return MODE_LEFT;}
    }
    return NONE;
} 



void vMainMenuTask()
{

    int startingLevel = 0;
    int highScore = 0;
    enum gameMode {SINGLE_PLAYER, MULTIPLAYER};
    enum gameMode Mode;
    char level[10];
    TickType_t last_change = 0;
    while(1)
    {
        if (DrawSignal)
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE)
            {
                if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE)
                {
                    tumDrawLoadedImage(mainmenu_background, 0,0);
                    tumDrawText(level, 365, 340, White);
                    tumDrawText("2000", 308, 72, White);
                    xSemaphoreGive(ScreenLock);
                }
		        tumEventFetchEvents(FETCH_EVENT_BLOCK | FETCH_EVENT_NO_GL_CHECK);
                //xGetButtonInput(); // Update global input

                switch (buttonPress(&last_change))
                {
                case START:
                    if(xSemaphoreTake(GameEngineLock, portMAX_DELAY) == pdTRUE){
                        printf("took lock\n");
                        mGame.InitGame(&mGame);
                        mGame.mGrid->ResetGrid(mGame.mGrid);
                        xSemaphoreGive(GameEngineLock);
                                                printf("gave lock\n");

                    }
                    xQueueSend(StateQueue, &next_state_signal, 0);
                    break;
                case LEVEL_LEFT:
                    if(startingLevel>0)
                        startingLevel = (startingLevel %29 )-1;
                    break;
                case LEVEL_RIGHT:
                    startingLevel= (startingLevel %29) +1 ;
                    break;
                case MODE_LEFT:
                    Mode--;
                case MODE_RIGHT:
                    Mode++;
                default:
                    break;
                }

                sprintf(level, "%d", startingLevel);

            }

    }
}


static bool isNewGamePressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 245 && x <= 400)
    {
        if(y >= 335 && y <= 365)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}

void vPauseTask()
{
    while(1)
    {
        if (DrawSignal)
        {
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE)
            {
                tumDrawLoadedImage(pause_background, 0,0);
            }

            if(isStartPressed())
            {
                xQueueSend(StateQueue, &prev_state_signal, 0);  //Go back to game
            }

            if(isNewGamePressed())
            {
                xQueueSend(StateQueue, &next_state_signal, 0);  //Go to main menu
            }
        }
    }

}

