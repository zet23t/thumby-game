#ifndef __GAME_CHARACTER_H__
#define __GAME_CHARACTER_H__

#include <inttypes.h>
#include "game.h"
void Character_update(Character *character, RuntimeContext *ctx, TE_Img *img, float tx, float ty, int8_t dirX, int8_t dirY);

#endif // __GAME_CHARACTER_H__