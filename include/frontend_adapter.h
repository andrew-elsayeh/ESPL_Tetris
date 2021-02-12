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