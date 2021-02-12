#include "grid.h"


/* 
 * Zeroes out the grid 
 */
void static ResetGrid(Grid_t *grid)
{
	for (int i = 0; i < GRID_WIDTH; i++) {
		for (int j = 0; j < GRID_HEIGHT; j++) {
			grid->mGrid[i][j] = EMPTY;
		}
	}

    grid->mRemovedLineCount = 0;
}

/* 
 * Merges the given tetrimino into the the grid
 */
void static MergeTetrimino(Grid_t *grid, int pX, int pY, int pTetrimino, int pRotation)
{
    // Store each block of the tetrimino into the grid
    for (int i1 = pX, i2 = 0; i1 < pX + TETRIMINO_SIDE; i1++, i2++)
    {
        for (int j1 = pY, j2 = 0; j1 < pY + TETRIMINO_SIDE; j1++, j2++)
        {   
            // Store only the blocks of the tetrimino that are not holes
            if (grid->pmTetriminos->prvGetBlock (pTetrimino, pRotation, j2, i2) != 0)      
                grid->mGrid[i1][j1] = grid->pmTetriminos->prvGetBlock (pTetrimino, pRotation, j2, i2);    
        }
    }
}

/* 
 * Returns true if the game is over, else false
 */
static bool IsGameOver(Grid_t *grid)
{
    //If the first line has blocks, then, game over
    for (int i = 0; i < GRID_WIDTH; i++)
    {
        if (grid->mGrid[i][0] != EMPTY) return true;
    }
 
    return false;
}

/* 
 * Removes one specific line from the grid and moves the rest one down
 */
static void DeleteLine(Grid_t *grid, int pY)
{
    for (int j = pY; j > 0; j--)
    {
        for (int i = 0; i < GRID_WIDTH; i++)
        {
            grid->mGrid[i][j] = grid->mGrid[i][j-1];
        }
    } 
}

/*
 * Original nintendo scoring system
 */
static void increaseScore (Grid_t *grid, int numRemovedLines)
{
    switch (numRemovedLines)
    {
    case 1:
        grid->mScore = grid->mScore + 40*(grid->mLevel+1);
        break;
    case 2:
        grid->mScore = grid->mScore + 100*(grid->mLevel+1);
        break;
    case 3:
        grid->mScore = grid->mScore + 300*(grid->mLevel+1);
        break;    
    case 4:
        grid->mScore = grid->mScore + 1200*(grid->mLevel+1);
        break;
    default:
        break;
    }
}

/* 
 * Checks for full lines and deletes them, and increases the score accordingly
 */
static void RemoveFullLines (Grid_t *grid)
{
    int mRemovedCount = 0;
    for (int j = 0; j < GRID_HEIGHT; j++)
    {
        int i = 0;
        while (i < GRID_WIDTH)
        {
            if (grid->mGrid[i][j] == EMPTY) break;
            i++;
        }
 
        if (i == GRID_WIDTH) 
        {
            DeleteLine (grid, j);
            mRemovedCount++;
            grid->mRemovedLineCount++;
            increaseScore(grid, mRemovedCount);
            vTaskDelay(300);
        }

    }

}

/* 
 * Checks if a specific block in the grid is free
 */
static bool IsFreeBlock (Grid_t *grid, int pX, int pY)
{
    if (grid->mGrid [pX][pY] == EMPTY) return true; else return false;
}

/* 
 * Returns the x coordinate in pixels of a block given its position in blocks
 */
static int GetXPosInPixels (Grid_t *grid, int pPos)
{
    return  ( ( BOARD_POSITION - (BLOCK_SIZE * (GRID_WIDTH / 2)) ) + (pPos * BLOCK_SIZE) );
}

/* 
 * Returns the y coordinate in pixels of a block given its position in blocks
 */
static int GetYPosInPixels (Grid_t *grid, int pPos)
{
    return ( (grid->mGameScreenHeight - (BLOCK_SIZE * GRID_HEIGHT)) + (pPos * BLOCK_SIZE) );
}

/* 
 * Collision detection in a grid. Checks if a a given tetrimino can be put in a specific position
 */
bool IsPossibleMovement(Grid_t *grid, int pX, int pY, int pTetrimino,
			int pRotation)
{
	for (int i1 = pX, i2 = 0; i1 < pX + TETRIMINO_SIDE; i1++, i2++) {
		for (int j1 = pY, j2 = 0; j1 < pY + TETRIMINO_SIDE;
		     j1++, j2++) {
			if (i1 < 0 || i1 > GRID_WIDTH - 1 ||
			    j1 > GRID_HEIGHT - 1) {
				if (grid->pmTetriminos->prvGetBlock(
					    pTetrimino, pRotation, j2, i2) != 0)
					return 0;
			}

			if (j1 >= 0) {
				if ((grid->pmTetriminos->prvGetBlock(
					     pTetrimino, pRotation, j2, i2) !=
				     0) &&
				    (!IsFreeBlock(grid, i1, j1)))
					return false;
			}
		}
	}
	return true;
}

/**
 * @brief Creates and initializes a grid object, bindes methods and attributes
 *
 */
void grid_init(Grid_t *grid, Tetrimino_t *pTetriminos, int pScreenHeight)
{
    grid->mGameScreenHeight = pScreenHeight;
    grid->pmTetriminos = pTetriminos;
    grid->IsPossibleMovement = IsPossibleMovement;
    grid->ResetGrid = ResetGrid;
    grid->MergeTetrimino = MergeTetrimino;
    grid->IsGameOver = IsGameOver;
    grid->DeleteLine = DeleteLine;
    grid->RemoveFullLines = RemoveFullLines;
    grid->IsFreeBlock = IsFreeBlock;
    grid->GetXPosInPixels = GetXPosInPixels;
    grid->GetYPosInPixels = GetYPosInPixels;

}