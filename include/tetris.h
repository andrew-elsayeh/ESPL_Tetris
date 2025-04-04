/******************************************************************************
* File Name: tetris.h
*
*
* Description: Tetris is the highest level object of the game engine
* It is primarily responisble for rendering the game 
* 
*******************************************************************************/

#pragma once

#include<stdlib.h>


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tetrimino.h"
#include "grid.h"
#include "frontend_adapter.h"
#include <time.h>



#define TetrisBlue (unsigned int)(0x0033cc) //A blue that blends in better with the color theme

typedef struct Tetris Tetris_t;

struct Tetris
{
    void (*DrawScene) (Tetris_t *tetris);
    void (*CreateNewPiece) (Tetris_t *tetris);
     
    int mPosX, mPosY;                   // Position of the tetrimino that is falling down
    int mShadowPosX, mShadowPosY;
    int mTetrimino, mRotation;          // Type and rotation of the tetrimino that is falling down

    int mGameScreenHeight;              // Screen height in pixels
    int mNextPosX, mNextPosY;       // Position of the next tetrimino
    int mNextTetrimino, mNextRotation;  // Kind and rotation of the next tetrimino

    bool mMultiplayer; //Flag for storing whether the game in multiplayer mode or not
 
    Grid_t  *mGrid;
    Tetrimino_t *pmTetriminos;
    FrontendAdapter_t *mFrontendAdapter;
 
    int (*GetRand) (int pA, int pB);
    void (*InitGame) (Tetris_t *tetris, int startingLevel, bool Multiplayer, int Algorithm);
    void (*DrawShadowPiece) (Tetris_t *tetris, int pX, int pY, int pPiece, int pRotation);
    void (*DrawPiece) (Tetris_t *tetris, int pX, int pY, int pPiece, int pRotation);
    void (*DrawBoard) (Tetris_t *tetris);
    void (*HardDrop) (Tetris_t *tetris);
    void (*calculateShadowPiece) (Tetris_t *tetris);
};

/**
 * @brief Initializes a tetris object
 *
 */
void tetris_init(Tetris_t *tetris, Grid_t *pBoard, Tetrimino_t *pTetriminos, FrontendAdapter_t *pFrontendAdapter, int pScreenHeight);