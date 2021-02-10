#pragma once

#include<stdlib.h>

#include "tetrimino.h"
#include "grid.h"
#include "frontend_adapter.h"
#include <time.h>


#define WAIT_TIME 250 // Number of milliseconds that the piece remains before going 1 block down */


typedef struct Tetris Tetris_t;

struct Tetris
{
    void (*DrawScene) (Tetris_t *tetris);
    void (*CreateNewPiece) (Tetris_t *tetris);
     
    int mPosX, mPosY;               // Position of the tetrimino that is falling down
    int mTetrimino, mRotation;          // Type and rotation of the tetrimino that is falling down

    int mGameScreenHeight;              // Screen height in pixels
    int mNextPosX, mNextPosY;       // Position of the next tetrimino
    int mNextTetrimino, mNextRotation;  // Kind and rotation of the next tetrimino

 
    Grid_t  *mGrid;
    Tetrimino_t *pmTetriminos;
    FrontendAdapter_t *mFrontendAdapter;
 
    int (*GetRand) (int pA, int pB);
    void (*InitGame) (Tetris_t *tetris);
    void (*DrawPiece) (Tetris_t *tetris, int pX, int pY, int pPiece, int pRotation);
    void (*DrawBoard) (Tetris_t *tetris);
};

/**
 * @brief Initializes a tetris object
 *
 */
void tetris_init(Tetris_t *tetris, Grid_t *pBoard, Tetrimino_t *pTetriminos, FrontendAdapter_t *pFrontendAdapter, int pScreenHeight);