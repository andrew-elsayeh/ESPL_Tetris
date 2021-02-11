#include "TUM_Draw.h"
#include "TUM_Event.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdio.h>
#include <stdbool.h>

#include "tetris.h"

bool isPausePressed();
