#pragma once

#include "tetrimino.h"
#include <stdbool.h>
#include "TUM_Draw.h"
#include "FreeRTOS.h"
#include "task.h"

// =============================================================================
// Definitions
// =============================================================================
#define BOARD_POSITION SCREEN_WIDTH/2   // Position of board on the screen
#define GRID_HEIGHT 20                  // Grid height in blocks
#define GRID_WIDTH 10                   // Grid width in blocks 
#define BLOCK_SIZE 22                   // Width and Height in pixels of each block of a piece
#define TETRIMINO_SIDE 5                // Number of horizontal and vertical blocks of a matrix piece
#define FRAMERATE_PERIOD 20             //Game runs at 50 FPS, needed to calculate game gravity

enum { EMPTY, YELLOW, CYAN, ORANGE, BLUE, RED, GREEN, PURPLE };   //Possibilities of entries in the grid

typedef struct Grid Grid_t;

static const int waitTime[30 /* levels*/] = 
{
    36*FRAMERATE_PERIOD,
    32*FRAMERATE_PERIOD,
    29*FRAMERATE_PERIOD,
    25*FRAMERATE_PERIOD,
    22*FRAMERATE_PERIOD,
    18*FRAMERATE_PERIOD,
    15*FRAMERATE_PERIOD,
    11*FRAMERATE_PERIOD,
    7*FRAMERATE_PERIOD,
    5*FRAMERATE_PERIOD,
    4*FRAMERATE_PERIOD,
    4*FRAMERATE_PERIOD,
    4*FRAMERATE_PERIOD,
    3*FRAMERATE_PERIOD,
    3*FRAMERATE_PERIOD,
    3*FRAMERATE_PERIOD,
    2*FRAMERATE_PERIOD,
    2*FRAMERATE_PERIOD,
    2*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD,
    1*FRAMERATE_PERIOD
};

// =============================================================================
// Grid Object Struture
// =============================================================================
struct Grid
{
    //Methods
    int (*GetXPosInPixels)         (Grid_t *grid, int pPos);
    int (*GetYPosInPixels)         (Grid_t *grid, int pPos);
    int (*waitTimeinMS)            (Grid_t *grid);
    bool (*IsFreeBlock)            (Grid_t *grid, int pX, int pY);
    bool (*IsPossibleMovement)     (Grid_t *grid, int pX, int pY, int pTetrimino, int pRotation);
    void (*MergeTetrimino)         (Grid_t *grid, int pX, int pY, int pTetrimino, int pRotation);
    void (*RemoveFullLines)        (Grid_t *grid);
    bool (*IsGameOver)             (Grid_t *grid);
    void (*ResetGrid)              (Grid_t *grid);
    void (*DeleteLine)             (Grid_t *grid, int pY);
    void (*updateLevel)            (Grid_t *grid);


 
    //Members and Attributes
    int          mGameScreenHeight;
    int          mGrid [GRID_WIDTH][GRID_HEIGHT];       // Grid that contains the pieces
    int          mRemovedLineCount, mScore, mLevel;     //Used to monitor game progress
    int          mHighScore;
    int          mStartngLines;                         //Number of lines before at first level up
    bool         startLevelingUp;                       //Used for managing leveling up
    Tetrimino_t *pmTetriminos;
};
 
/**
 * @brief Creates and initializes a booard object
 * @return a Grid_t Object
 *
 */
void grid_init(Grid_t *grid, Tetrimino_t *pTetriminos, int pScreenHeight);