#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

#include "TE_Image.h"
#include "TE_Font.h"
#include "game_renderobjects.h"

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
#define SPRITE_UI_SHIELD 17
#define SPRITE_UI_SWORD 18
#define SPRITE_UI_BATTLE_BAR 19
#define SPRITE_TEXT_CRIT 20
#define SPRITE_TEXT_MISS 21
#define SPRITE_TEXT_OUCH 22
#define SPRITE_TEXT_OW 23
#define SPRITE_TEXT_OOF 24
#define SPRITE_TEXT_BANG 25
#define SPRITE_TREE_FOLLIAGE_1 26
#define SPRITE_TREE_FOLLIAGE_2 27
#define SPRITE_TREE_FOLLIAGE_3 28
#define SPRITE_TREE_FOLLIAGE_4 29
#define SPRITE_TREE_FOLLIAGE_SMALL_1 30
#define SPRITE_TREE_FOLLIAGE_SMALL_2 31
#define SPRITE_TREE_FOLLIAGE_SMALL_3 32
#define SPRITE_TREE_FOLLIAGE_SMALL_4 33
#define SPRITE_FLAT_ARROW_RIGHT 34
#define SPRITE_FLAT_ARROW_LEFT 35
#define SPRITE_FLAT_ARROW_UP 36
#define SPRITE_FLAT_ARROW_DOWN 37
#define SPRITE_FLAT_ARROW_UP_LEFT 38
#define SPRITE_FLAT_ARROW_UP_RIGHT 39
#define SPRITE_FLAT_ARROW_DOWN_LEFT 40
#define SPRITE_FLAT_ARROW_DOWN_RIGHT 41

#define FONT_MEDIUM 0
#define FONT_LARGE 1
#define FONT_TINY 2

#define ANIMATION_HAHAHA_RIGHT 1
#define ANIMATION_HAHAHA_LEFT 2
#define ANIMATION_STAFF_ATTACK 3
#define ANIMATION_STAFF_IDLE 4
#define ANIMATION_STAFF_ATTACK_HIT 5
#define ANIMATION_STAFF_AIM 6

#define RENDER_PREFAB_TREE 1

TE_Img *GameAssets_getAtlasImg(void);
TE_Sprite GameAssets_getSprite(uint8_t index);
TE_Font GameAssets_getFont(uint8_t index);
int GameAssets_drawAnimation(uint8_t index, TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx);
RenderPrefab* GameAssets_getRenderPrefab(uint8_t index, uint8_t variant);

#endif