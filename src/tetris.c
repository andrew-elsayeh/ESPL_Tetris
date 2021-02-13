#include "tetris.h"
#include "multiplayer.h"
#include "FreeRTOS.h"
#include "queue.h"

extern SemaphoreHandle_t GameEngineLock;
extern QueueHandle_t    StateQueue;

extern QueueHandle_t PieceQueue;

/*                                   
Returns a random number between 2 integers
 
*/
int GetRand (int pA, int pB)
{
    return rand () % (pB - pA + 1) + pA;
}

/*                                   
Returns a random number between 2 integers
 
*/
int GetRandRotation (int pA, int pB)
{
    return rand () % (pB - pA + 1) + pA;
}


/**
 * Starts a new game
 */
void InitGame(Tetris_t *tetris, int startingLevel, bool Multiplayer, int Algorithm)
{
    if(Multiplayer)
    {
        tetris->mMultiplayer = true;
        startMultiplayer();
        assertBinaryRunning();
        resetMultiplayer();
        setMode(Algorithm);
    }
    else{
        tetris->mMultiplayer = false;
    }
    srand ((unsigned int) time(NULL));

    tetris->mGrid->mScore = 0;
    tetris->mGrid->mLevel = startingLevel;
    
    if(tetris->mMultiplayer)
    {
        tetris->mTetrimino = getPiecefromUDB();
        tetris->mNextTetrimino = getPiecefromUDB();
    }
    else
    {
    tetris->mTetrimino        = GetRand (0, 6);
    tetris->mNextTetrimino      = GetRand (0, 6);
    }
 
    // First tetrimino
    tetris->mRotation       = GetRandRotation (0, 3);
    tetris->mPosX           = 2;
    tetris->mPosY           = -3;

    //  Next tetrimino
    tetris->mNextRotation   = GetRandRotation (0, 3);
    tetris->mNextPosX       = GRID_WIDTH + 3;
    tetris->mNextPosY       = 9;    

    tetris->mShadowPosY     = -3;

    //Caluclate Shadow Piece
    while (tetris->mGrid->IsPossibleMovement(tetris->mGrid, tetris->mPosX, tetris->mShadowPosY, tetris->mTetrimino, tetris->mRotation))
    {
        tetris->mShadowPosY++;
    }

}

/**
 *Creates a random tetrimino
 */
void CreateNewPiece(Tetris_t *tetris)
{
    // The new tetrimino
    tetris->mTetrimino          = tetris->mNextTetrimino;
    tetris->mRotation       = tetris->mNextRotation;

    tetris->mPosX           = 2;
    tetris->mPosY           = -3;

    tetris->mShadowPosY     = -3;

    //Caluclate Shadow Piece
    while (tetris->mGrid->IsPossibleMovement(tetris->mGrid, tetris->mPosX, tetris->mShadowPosY, tetris->mTetrimino, tetris->mRotation))
    {
        tetris->mShadowPosY++;
    }

    //Next tetrimino
    if(tetris->mMultiplayer)
    {
        tetris->mNextTetrimino = getPiecefromUDB();
    }
    else
    {
    tetris->mNextTetrimino      = GetRand (0, 6);
    }
    tetris->mNextRotation   = GetRandRotation (0, 3);
}

/**
 *Draws a single tetrimino
 */
void DrawPiece (Tetris_t *tetris, int pX, int pY, int pPiece, int pRotation)
{
    int mColor;               // Color of the block 
 
    // Obtain the position in pixel in the screen of the block we want to draw
    int mPixelsX = tetris->mGrid->GetXPosInPixels (tetris->mGrid, pX);
    int mPixelsY = tetris->mGrid->GetYPosInPixels (tetris->mGrid, pY);
 
    // Travel the matrix of blocks of the tetrimino and draw the blocks that are filled
    for (int i = 0; i < TETRIMINO_SIDE; i++)
    {
        for (int j = 0; j < TETRIMINO_SIDE; j++)
        {
            // Get the type of the block and draw it with the correct color
            switch (tetris->pmTetriminos->prvGetBlock (pPiece, pRotation, j, i))
            {
                //Official Tetris Colors according to 
                //https://tetris.fandom.com/wiki/Tetris_Guideline
                case 1: mColor = Yellow; break;  
                case 2: mColor = Cyan; break;   
                case 3: mColor = Orange; break;
                case 4: mColor = TetrisBlue; break;
                case 5: mColor = Red; break;
                case 6: mColor = Green; break;
                case 7: mColor = Purple; break;
            }
             
            if (tetris->pmTetriminos->prvGetBlock (pPiece, pRotation, j, i) != 0)
                tetris->mFrontendAdapter->DrawRectangle   (mPixelsX + i * BLOCK_SIZE, 
                                    mPixelsY + j * BLOCK_SIZE, 
                                    (mPixelsX + i * BLOCK_SIZE) + BLOCK_SIZE - 1, 
                                    (mPixelsY + j * BLOCK_SIZE) + BLOCK_SIZE - 1, 
                                    mColor);
        }
    }
}

/**
 *Draws a single tetrimino
 */
void DrawShadowPiece (Tetris_t *tetris, int pX, int pY, int pPiece, int pRotation)
{
    int mColor;               // Color of the block 
 
    // Obtain the position in pixel in the screen of the block we want to draw
    int mPixelsX = tetris->mGrid->GetXPosInPixels (tetris->mGrid, pX);
    int mPixelsY = tetris->mGrid->GetYPosInPixels (tetris->mGrid, pY);
 
    // Travel the matrix of blocks of the tetrimino and draw the blocks that are filled
    for (int i = 0; i < TETRIMINO_SIDE; i++)
    {
        for (int j = 0; j < TETRIMINO_SIDE; j++)
        {
            // Get the type of the block and draw it with the correct color
            switch (tetris->pmTetriminos->prvGetBlock (pPiece, pRotation, j, i))
            {
                //Official Tetris Colors according to 
                //https://tetris.fandom.com/wiki/Tetris_Guideline
                case 1: mColor = Yellow; break;  
                case 2: mColor = Cyan; break;   
                case 3: mColor = Orange; break;
                case 4: mColor = TetrisBlue; break;
                case 5: mColor = Red; break;
                case 6: mColor = Green; break;
                case 7: mColor = Purple; break;
            }
             
            if (tetris->pmTetriminos->prvGetBlock (pPiece, pRotation, j, i) != 0)
                tetris->mFrontendAdapter->DrawShadowRectangle   (mPixelsX + i * BLOCK_SIZE, 
                                    mPixelsY + j * BLOCK_SIZE, 
                                    (mPixelsX + i * BLOCK_SIZE) + BLOCK_SIZE - 1, 
                                    (mPixelsY + j * BLOCK_SIZE) + BLOCK_SIZE - 1, 
                                    mColor);
        }
    }
}

/**
 *Draws the tetriminos stored in the grid
 */
void DrawBoard (Tetris_t *tetris)
{

    int mColor;               // Color of the block 
 
    // Calculate the limits of the grid in pixels  
    int mX1 = BOARD_POSITION - (BLOCK_SIZE * (GRID_WIDTH / 2)) - 1;
    int mY = tetris->mGameScreenHeight - (BLOCK_SIZE * GRID_HEIGHT);
     

    // Drawing the blocks that are already stored in the grid
    mX1 += 1;
    for (int i = 0; i < GRID_WIDTH; i++)
    {
        for (int j = 0; j < GRID_HEIGHT; j++)
        {   
            // Check if the block is filled, if so, draw it
            if (!tetris->mGrid->IsFreeBlock(tetris->mGrid, i, j))  
            {
                switch (tetris->mGrid->mGrid[i][j])
                {
                    //Official Tetris Colors according to 
                    //https://tetris.fandom.com/wiki/Tetris_Guideline
                    case 1: mColor = Yellow; break;  
                    case 2: mColor = Cyan; break;   
                    case 3: mColor = Orange; break;
                    case 4: mColor = TetrisBlue; break;
                    case 5: mColor = Red; break;
                    case 6: mColor = Green; break;
                    case 7: mColor = Purple; break;
                }
                tetris->mFrontendAdapter->DrawRectangle ( mX1 + i * BLOCK_SIZE, 
                        mY + j * BLOCK_SIZE, 
                        (mX1 + i * BLOCK_SIZE) + BLOCK_SIZE - 1, 
                        (mY + j * BLOCK_SIZE) + BLOCK_SIZE - 1, 
                        mColor);
            }


        }
    }   
}

/**
 *Draws the entire game
 */
void DrawScene (Tetris_t *tetris)
{
    DrawBoard (tetris);                                                                                         // Draw the blocks stored in the grid
    DrawPiece (tetris, tetris->mPosX, tetris->mPosY, tetris->mTetrimino, tetris->mRotation);                    // Draw the playing tetrimino
    DrawShadowPiece (tetris, tetris->mPosX, tetris->mShadowPosY, tetris->mTetrimino, tetris->mRotation);        // Draw Shadow Piece
    DrawPiece (tetris, tetris->mNextPosX, tetris->mNextPosY, tetris->mNextTetrimino, tetris->mNextRotation);    // Draw the next tetrimino
}


void HardDrop (Tetris_t *tetris)
{
    while (tetris->mGrid->IsPossibleMovement(
        tetris->mGrid,
        tetris->mPosX, tetris->mPosY, tetris->mTetrimino,
        tetris->mRotation)) {
        tetris->mPosY++;
    }

    tetris->mGrid->MergeTetrimino(tetris->mGrid ,tetris->mPosX, tetris->mPosY - 1,
            tetris->mTetrimino, tetris->mRotation);

    tetris->mGrid->RemoveFullLines(tetris->mGrid);

    tetris->mGrid->updateLevel(tetris->mGrid);


    if (tetris->mGrid->IsGameOver(tetris->mGrid)) {
        return;
    }

    tetris->CreateNewPiece(tetris);
}



 void calculateShadowPiece(Tetris_t *tetris)
 {
    tetris->mShadowPosY = -3;   //reset position
    while (tetris->mGrid->IsPossibleMovement(tetris->mGrid, tetris->mPosX, tetris->mShadowPosY, tetris->mTetrimino, tetris->mRotation))
    {
        tetris->mShadowPosY++;
    }
 }



void tetris_init(Tetris_t *tetris, Grid_t *pGrid, Tetrimino_t *pTetriminos, FrontendAdapter_t *pFrontendAdapter, int pScreenHeight)
{
    tetris->mGrid = pGrid;
    tetris->pmTetriminos = pTetriminos;
    tetris->mFrontendAdapter = pFrontendAdapter;
    tetris->mGameScreenHeight = pScreenHeight;



    tetris->GetRand = GetRand;
    tetris->InitGame = InitGame;
    tetris->CreateNewPiece = CreateNewPiece;
    tetris->DrawPiece = DrawPiece;
    tetris->DrawBoard = DrawBoard;
    tetris->DrawScene = DrawScene;
    tetris->HardDrop = HardDrop;
    tetris->DrawShadowPiece = DrawShadowPiece;
    tetris->calculateShadowPiece = calculateShadowPiece;

}