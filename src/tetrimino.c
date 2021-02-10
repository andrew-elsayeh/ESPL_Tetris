#include "tetrimino.h"


/**
 * @brief Return the type of a block, the number refers to the type of tetrinmino
 * 1->O
 * 2->I
 * 3->L
 * 4->J
 * 5->N
 * 6->S
 * 7->T
 *
 * @param pTetrimino Tetrimino in question
 * @param pRotation: The rotation of the tetrimino
 * @param pX:   Horizontal position in blocks
 * @param pY:   Vertical position in blocks
 */
static uint16_t prvGetBlock (int pTetrimino, int pRotation, int pX, int pY)
{
    return pusTetriminos [pTetrimino][pRotation][pX][pY];
}



void tetrimino_init(Tetrimino_t *tetrimino)
{
    tetrimino->prvGetBlock = prvGetBlock;
}