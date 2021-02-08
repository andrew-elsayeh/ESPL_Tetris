// =============================================================================
// Functions and structurs for the Game Engine
// =============================================================================

#ifndef TETRIMINOS_H
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


// =============================================================================
// Definitions
// =============================================================================

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 22
#define BOARD_OFFSET_X 210
#define BOARD_OFFSET_Y 20

// =============================================================================
// Structures
// =============================================================================
typedef enum tetrimino_type{
    TETRIMINO_I,
    TETRIMINO_O,
    TETRIMINO_T,
    TETRIMINO_S,
    TETRIMINO_Z,
    TETRIMINO_J,
    TETRIMINO_L
} tetrimino_type;

typedef struct Piece_State
{
    tetrimino_type type;
    uint8_t    x_offset;
    uint8_t    y_offset;
    uint8_t    rotation;
}Piece_State_t;

typedef struct Game_State
{
    uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];
    Piece_State_t piece; 
} Game_State_t;

// =============================================================================
// Public Functions
// =============================================================================
void draw_cell(uint8_t row, uint8_t colomn, uint8_t color);



#define TETRIMINOS_H
#endif /* TETRIMINOS_H */
