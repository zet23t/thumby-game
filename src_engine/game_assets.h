#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

#include "TE_Image.h"
#include "TE_Font.h"

#define SPRITE_ROBIN_HEAD_FRONT 1
#define SPRITE_CART_WHEEL_SIDE 2
#define SPRITE_CART_SIDE 3
#define SPRITE_CART_GOLD 4
#define SPRITE_ARROW_RIGHT 5

#define FONT_MEDIUM 0
#define FONT_LARGE 1

TE_Sprite GameAssets_getSprite(uint8_t index);
TE_Font GameAssets_getFont(uint8_t index);

#endif