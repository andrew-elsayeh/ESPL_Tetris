#include "tetriminos.h"



void draw_cell(uint8_t row, uint8_t col, uint8_t color){
    uint16_t x_coordinate = col*22 + BOARD_OFFSET_X;
    uint16_t y_coordinate = row*22 +BOARD_OFFSET_Y ;
    tumDrawFilledBox(x_coordinate,y_coordinate, 20,20, Black);
}

void draw_board(Game_State_t* Game_State){
    for (int row = 0; row < BOARD_HEIGHT; row++)   //for each coloumn
    {
        for (int col = 0; col < BOARD_WIDTH; col++)  //for each row
        {
            if(Game_State->board[col][row])
            {
                draw_cell(row, col, Black);
            }
        }
        
    }
    
}
