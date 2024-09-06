#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

#include "TE_Image.h"
#include "TE_Font.h"

#define SPRITE_ROBIN_HEAD_FRONT 1
#define SPRITE_CART_WHEEL_SIDE 2
#define SPRITE_CART_SIDE 3
#define SPRITE_CART_GOLD 4
#define SPRITE_ARROW_RIGHT 5
#define SPRITE_POLE_TOP 6
#define SPRITE_ANIM_HAHAHA_R_F1 7
#define SPRITE_ANIM_HAHAHA_R_F2 8
#define SPRITE_ANIM_HAHAHA_R_F3 9
#define SPRITE_ANIM_HAHAHA_R_F4 10
#define SPRITE_ANIM_HAHAHA_L_F1 11
#define SPRITE_ANIM_HAHAHA_L_F2 12
#define SPRITE_ANIM_HAHAHA_L_F3 13
#define SPRITE_ANIM_STAFF_HIT_F1 14
#define SPRITE_ANIM_STAFF_HIT_F2 15
#define SPRITE_ANIM_STAFF_HIT_F3 16

#define FONT_MEDIUM 0
#define FONT_LARGE 1

#define ANIMATION_HAHAHA_RIGHT 1
#define ANIMATION_HAHAHA_LEFT 2
#define ANIMATION_STAFF_ATTACK 3
#define ANIMATION_STAFF_IDLE 4
#define ANIMATION_STAFF_ATTACK_HIT 5

TE_Sprite GameAssets_getSprite(uint8_t index);
TE_Font GameAssets_getFont(uint8_t index);
int GameAssets_drawAnimation(uint8_t index, TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx);

#endif