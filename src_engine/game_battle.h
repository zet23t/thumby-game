#ifndef GAME_BATTLE_H
#define GAME_BATTLE_H

#include "game.h"
#include "game_assets.h"

#include <inttypes.h>

typedef struct BattleAction BattleAction;

typedef struct BattleEntityState
{
    uint8_t id:4;
    uint8_t target:4;
    uint8_t nextTarget:4;
    uint8_t team:2;
    uint8_t characterType:4;
    uint8_t actionPoints:4;
    uint8_t hitpoints:4;
    uint8_t position:4;
    float actionTimer;
    const char *name;
    BattleAction *actionNTList;
} BattleEntityState;

typedef struct BattlePosition
{
    uint8_t x;
    uint8_t y;
} BattlePosition;

typedef struct BattleState
{
    BattleEntityState entities[8];
    uint8_t entityCount;
    BattlePosition positions[16];
    int8_t selectedAction:4;
    int8_t activatingAction:4;
    int8_t queuedEntityId;
    uint8_t queuedActionId:4;
    float timer;
} BattleState;

#define BATTLEACTION_ONACTIVATING_CONTINUE 0
#define BATTLEACTION_ONACTIVATING_DONE 1
#define BATTLEACTION_ONACTIVATING_CANCEL 2

#define BATTLEACTION_ONSELECTED_IGNORE 0
#define BATTLEACTION_ONSELECTED_ACTIVATE 1

typedef struct BattleMenuEntry
{
    const char *menuText;
    const char *columnText;
    uint8_t id;
} BattleMenuEntry;
#define BattleMenuEntryDef(menuText, columnText, id) ((BattleMenuEntry){menuText, columnText, id})

typedef struct BattleMenuWindow
{
    int16_t x, y;
    int16_t w, h;
    int16_t divX;
    int16_t divX2;
    int16_t lineHeight;
    uint32_t selectedColor;
} BattleMenuWindow;

typedef struct BattleMenu
{
    int8_t selectedAction;
    uint8_t entriesCount;
    BattleMenuEntry *entries;
} BattleMenu;

typedef struct BattleAction
{
    const char *name;
    uint8_t actionPointCosts;
    int8_t selectedAction;
    uint8_t (*onSelected)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);
    uint8_t (*onActivating)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);
    uint8_t (*onActivated)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);
} BattleAction;

void BattleMenuWindow_update(RuntimeContext *ctx, TE_Img *screen, BattleMenuWindow* window, BattleMenu *battleMenu);

#endif