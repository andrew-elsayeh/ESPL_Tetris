
#include "menu.h"

enum MainMenuButton {NONE, START, LEVEL_LEFT, LEVEL_RIGHT, MODE_RIGHT, MODE_LEFT, SINGLE, MULTI};

extern QueueHandle_t StateQueue;


extern image_handle_t gameplay_background;
extern image_handle_t menu_single_background;
extern image_handle_t menu_multiplayer_background;
extern image_handle_t pause_background;

extern const unsigned char next_state_signal;
extern const unsigned char prev_state_signal;

extern SemaphoreHandle_t DrawSignal;
extern SemaphoreHandle_t ScreenLock; 
extern SemaphoreHandle_t GameEngineLock;

extern Tetris_t mGame; 

// =============================================================================
// Simple functions that return true if a specific button is pressed
// =============================================================================
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
        if(y >= 415 && y <= 435)
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
        if(y >= 415 && y <= 435)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
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

static bool isMultiPressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 320 && x <= 400)
    {
        if(y >= 380 && y <= 410)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}

static bool isSinglePressed()
{
    int x = tumEventGetMouseX();
    int y = tumEventGetMouseY();
    int click = tumEventGetMouseLeft();
    if  (x >= 245 && x <= 320)
    {
        if(y >= 380 && y <= 410)
        {
            if(click)
            {
                return true;
            }
        }
    }
    return false;
}



static enum MainMenuButton buttonPress(TickType_t *lastButtonPress)
{
    if (xTaskGetTickCount() - *lastButtonPress  > 100)  //Basic Debouncing of Mouse Key
    {
        if(isStartPressed()) {*lastButtonPress = xTaskGetTickCount(); return START;}
        if(isLevelLeftPressed()){*lastButtonPress = xTaskGetTickCount();  return LEVEL_LEFT;}
        if(isLevelRightPressed()){*lastButtonPress = xTaskGetTickCount();  return LEVEL_RIGHT;}
        if(isModeLeftPressed()){*lastButtonPress = xTaskGetTickCount();  return MODE_RIGHT;}
        if(isModeRightPressed()){*lastButtonPress = xTaskGetTickCount();  return MODE_LEFT;}
        if(isSinglePressed()){*lastButtonPress = xTaskGetTickCount();  return SINGLE;}
        if(isMultiPressed()){*lastButtonPress = xTaskGetTickCount();  return MULTI;}
    }
    return NONE;
} 

#define MAX_LEVEL 29

void vMainMenuTask()
{

    int startingLevel = 0;
    int highScore = 0;
    enum gameMode {SINGLE_PLAYER, MULTIPLAYER};
    enum gameMode Mode = SINGLE_PLAYER;

    enum gameAlgorithm {FAIR, RANDOM, EASY, HARD, DETERMINISTIC};
    enum gameAlgorithm Algorithm = FAIR;
    char startingLevel_str[12];
    char highScore_str[12];
    int width;

    TickType_t last_change = 0;
    while(1)
    {
        if (DrawSignal)
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE)
            {
                if(xSemaphoreTake(GameEngineLock, portMAX_DELAY) == pdTRUE){
                    highScore = mGame.mGrid->mHighScore;
                    xSemaphoreGive(GameEngineLock);
                }

                if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE)
                {
                    
                    if(Mode == SINGLE_PLAYER) tumDrawLoadedImage(menu_single_background, 0,0);
                    
                    tumDrawText(startingLevel_str, 365, 340, White);
                    tumGetTextSize(highScore_str, &width, NULL);
                    tumDrawText(highScore_str, SCREEN_WIDTH/2- width/2, 72, White);

                    if (Mode == MULTIPLAYER)
                    {

                        tumDrawLoadedImage(menu_multiplayer_background, 0,0);
                        tumDrawText(startingLevel_str, 365, 340, White);
                        tumGetTextSize(highScore_str, &width, NULL);
                        tumDrawText(highScore_str, SCREEN_WIDTH/2- width/2, 72, White);
                        switch (Algorithm)
                        {
                        case FAIR:
                            tumDrawText("Fair", 345, 410, White);
                            break;
                        case RANDOM:
                            tumDrawText("Rand.", 345, 410, White);
                            break;
                        case EASY:
                            tumDrawText("Easy", 345, 410, White);
                            break;
                        case HARD:
                            tumDrawText("Hard", 345, 410, White);
                            break;
                        case DETERMINISTIC:
                            tumDrawText("Det.", 345, 410, White);
                            break;
                        default:
                            break;
                        }
                    }
                    

                    xSemaphoreGive(ScreenLock);
                }
		        tumEventFetchEvents(FETCH_EVENT_BLOCK | FETCH_EVENT_NO_GL_CHECK);
                //xGetButtonInput(); // Update global input

                switch (buttonPress(&last_change))
                {
                case START:
                    if(xSemaphoreTake(GameEngineLock, portMAX_DELAY) == pdTRUE){
                        mGame.InitGame(&mGame, startingLevel);
                        mGame.mGrid->ResetGrid(mGame.mGrid);
                        xSemaphoreGive(GameEngineLock);
                    }
                    xQueueSend(StateQueue, &next_state_signal, 0);
                    break;
                case LEVEL_LEFT:
                    if(startingLevel>0)
                        startingLevel = (startingLevel % MAX_LEVEL )-1;
                    break;
                case LEVEL_RIGHT:
                    startingLevel= (startingLevel % MAX_LEVEL) +1 ;
                    break;
                case MODE_LEFT:
                    Algorithm = (Algorithm + 1) %5;
                    break;
                case MODE_RIGHT:
                    Algorithm = (Algorithm - 1) %5;
                    break;
                case SINGLE:
                    Mode = SINGLE_PLAYER;
                    break;
                case MULTI:
                    Mode = MULTIPLAYER;
                    break;
                default:
                    break;
                }
                sprintf(startingLevel_str, "%d", startingLevel);
                sprintf(highScore_str, "%d", highScore);

            }

    }
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

