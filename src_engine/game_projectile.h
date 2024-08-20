#ifndef __GAME_PROJECTILE_H__
#define __GAME_PROJECTILE_H__

#include <inttypes.h>

#include "game.h"

void Projectile_spawn(float x, float y, float vx, float vy, uint32_t color);
void Projectiles_update(Projectile *projectile, RuntimeContext *ctx, TE_Img *img);

#endif // __GAME_PROJECTILE_H__