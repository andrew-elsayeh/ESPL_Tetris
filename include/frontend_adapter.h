/******************************************************************************
* File Name: frontend_adapter.h
*
*
* Description: This "Object" is the interface between the game engine and TUMDraw
* It's used to detach the game logic from drawing, so that the code can be modified 
* to use a different library. For example to run the program on a real development
* board instead of the emulator
* 
*******************************************************************************/
#pragma once

#include "TUM_Draw.h"
#include "tetrimino.h"

typedef struct frontend_adapter FrontendAdapter_t;

extern image_handle_t gameplay_background;

struct frontend_adapter{
  void (*DrawRectangle)(int pX1, int pY1, int pX2, int pY2, int color);
  void (*DrawShadowRectangle)(int pX1, int pY1, int pX2, int pY2, int color);
  void (*ClearScreen)();
};

void frontendAdapter_init(FrontendAdapter_t *frontend_adapter);