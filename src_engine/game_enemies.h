#ifndef __GAME_ENEMIES_H__
#define __GAME_ENEMIES_H__

#include <inttypes.h>
#include "game.h"

#define ENEMY_RADIUS 4
#define ENEMY_RECT_X -5
#define ENEMY_RECT_Y -3
#define ENEMY_RECT_WIDTH 9
#define ENEMY_RECT_HEIGHT 12

void Enemies_init();
int Enemies_spawn(uint8_t id, int type, int16_t x, int16_t y);
void Enemies_setTarget(uint8_t id, float x, float y);
void Enemies_update(RuntimeContext *ctx, TE_Img *img);
int Enemies_raycastPoint(float x, float y);
void Enemies_setItem(uint8_t id, int8_t leftItemIndex, int8_t rightItemIndex);
int Enemies_getPosition(uint8_t id, float *outX, float *outY);
void Enemies_setHealth(uint8_t id, float health);
int Enemies_isAlive(uint8_t id);
int Enemy_takeDamage(Enemy *enemy, float damage, float srcVx, float srcVy, RuntimeContext *ctx, TE_Img *screen);
Enemy* Enemies_getEnemy(uint8_t id);

#endif