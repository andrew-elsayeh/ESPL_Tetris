#include "frontend_adapter.h"

void DrawRectangle(int pX1, int pY1, int pX2, int pY2, int color)
{
    int width = pX2 - pX1;
    int height = pY2 - pY1;

    tumDrawFilledBox(pX1,pY2, width,height, 0x0c1748);  //Draw border of rectangle with the color of the background
    tumDrawFilledBox(pX1,pY2, width-2,height-2, color); //Draw the Rectangle itself with - a 2px border

}

void ClearScreen(){
    tumDrawLoadedImage(gameplay_background, 0,0);
}

void frontendAdapter_init(FrontendAdapter_t *frontend_adapter)
{

    frontend_adapter->DrawRectangle = DrawRectangle;
    frontend_adapter->ClearScreen = ClearScreen;
}