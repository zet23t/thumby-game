#ifndef __GAME_PLAYER_H__
#define __GAME_PLAYER_H__

#include "game.h"

void Player_setWeapon(uint8_t weaponIndex);
void Player_update(Player *player, Character *playerCharacter, RuntimeContext *ctx, TE_Img *img);
void Player_setInputEnabled(uint8_t enabled);

#endif // __GAME_PLAYER_H__