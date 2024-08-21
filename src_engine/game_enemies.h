#ifndef __GAME_ENEMIES_H__
#define __GAME_ENEMIES_H__

#include <inttypes.h>
#include "game.h"

#define ENEMY_RADIUS 4
#define ENEMY_RECT_X -5
#define ENEMY_RECT_Y -3
#define ENEMY_RECT_WIDTH 9
#define ENEMY_RECT_HEIGHT 12

int Enemies_spawn(int type, int x, int y);
void Enemies_update(RuntimeContext *ctx, TE_Img *img);
int Enemies_raycastPoint(float x, float y);

#endif