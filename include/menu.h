/******************************************************************************
* File Name: menu.c
*
*
* Description: This file contains the function and task functions needed for 
* the main menu, and the pause screen
* Most of its functions are static. Meaning they can only be used by the menu. 
* Only isPausePressed() is exposed to allow usage during gameplay
* 
*******************************************************************************/
#include "TUM_Draw.h"
#include "TUM_Event.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdio.h>
#include <stdbool.h>

#include "tetris.h"


/*
 * Returns true if the pause button is pressed 
 */
bool isPausePressed();
