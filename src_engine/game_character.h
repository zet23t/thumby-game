#ifndef __GAME_CHARACTER_H__
#define __GAME_CHARACTER_H__

#include <inttypes.h>
#include "game.h"
void Character_update(Character *character, RuntimeContext *ctx, TE_Img *img, float tx, float ty, int8_t dirX, int8_t dirY);
int Character_raycastCircle(Character *character, int16_t px, int16_t py, int16_t radius, int16_t *outCenterX, int16_t *outCenterY, int16_t *outRadius);
void Character_toBaseF(Character *character, float *x, float *y);
void Character_fromBaseF(Character *character, float *x, float *y);

#endif // __GAME_CHARACTER_H__