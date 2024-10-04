#ifndef __GAME_BATTLE_ACTIONS_H__
#define __GAME_BATTLE_ACTIONS_H__

#include "game_battle.h"

BattleAction BattleAction_Thrust();
BattleAction BattleAction_ChangeTarget();
BattleAction BattleAction_Strike();
BattleAction BattleAction_Parry();
BattleAction BattleAction_Insult(const char **insults);

#endif