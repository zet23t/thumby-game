#ifndef GAME_BATTLE_H
#define GAME_BATTLE_H

#include "game.h"
#include "game_assets.h"

#include <inttypes.h>

typedef struct BattleAction BattleAction;
typedef struct BattleMenu BattleMenu;

typedef struct BattleMenuEntry
{
    const char *menuText;
    const char *columnText;
    TE_Vector2_s16 textSize;
    float time;
    float textScrollX;
    BattleAction *action;
    uint8_t id;
} BattleMenuEntry;
#define BattleMenuEntryDef(menuText_, columnText_, id_) ((BattleMenuEntry){.menuText=menuText_, .columnText = columnText_, .id =id_})

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
    float selectedActionY;
} BattleMenu;

typedef struct BattleEntityState
{
    uint8_t id:4;
    uint8_t target:4;
    uint8_t nextTarget:4;
    uint8_t team:2;
    uint8_t characterType:4;
    uint8_t actionPoints:4;
    uint8_t hitpoints:4;
    uint8_t maxHitpoints:4;
    uint8_t position:4;
    uint16_t lastActionAtCounter;
    float actionTimer;
    const char *name;
    BattleAction *actionNTList;
} BattleEntityState;

typedef struct BattlePosition
{
    uint8_t x;
    uint8_t y;
} BattlePosition;

#define BATTLESTATE_MAX_ENTITIES 8

typedef struct BattleState
{
    BattleEntityState entities[BATTLESTATE_MAX_ENTITIES];
    BattleAction* activeActions[BATTLESTATE_MAX_ENTITIES];
    BattleMenu menu;
    BattleMenuWindow menuWindow;
    uint8_t entityCount;
    uint16_t actionCounter;
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

#define BATTLEACTION_ONACTIVATED_CONTINUE 0
#define BATTLEACTION_ONACTIVATED_DONE 1
#define BATTLEACTION_ONACTIVATED_ISACTIVE 2

#define BATTLEACTION_ONACTIVE_CONTINUE 0
#define BATTLEACTION_ONACTIVE_DONE 1

#define BATTLEACTION_ONSELECTED_IGNORE 0
#define BATTLEACTION_ONSELECTED_ACTIVATE 1

typedef struct BattleAction
{
    const char *name;
    uint8_t actionPointCosts;
    int8_t selectedAction;
    uint8_t statusFlags;
    void *userData;
    float actionTimer;
    BattleMenu *menu;

    //## Battle action callbacks

    // Called during the action is active in selection
    // return BATTLEACTION_ONSELECTED_ACTIVATE or BATTLEACTION_ONSELECTED_IGNORE to signal the action is ready to run
    uint8_t (*onSelected)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);

    // Called when the action is being selected for activation; used for showing options or preparing the action
    // return BATTLEACTION_ONACTIVATING_DONE or BATTLEACTION_ONACTIVATING_CONTINUE or BATTLEACTION_ONACTIVATING_CANCEL
    uint8_t (*onActivating)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);

    // Called when the action is starting to run. Return 1 to signal the action is done
    // - return BATTLEACTION_ONACTIVATED_CONTINUE to signal the action is still running, 
    // - BATTLEACTION_ONACTIVATED_DONE for signalling the action is done, 
    // - BATTLEACTION_ONACTIVATED_ISACTIVE for signalling the action is done but to be flagged as active action
    uint8_t (*onActivated)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);
    
    // Called when the action is in the active action list of the battle state while the combat continues and other units are acting
    // - return BATTLEACTION_ONACTIVE_CONTINUE to signal the action is still active
    // - BATTLEACTION_ONACTIVE_DONE for signalling the action is done and to be removed as active action
    uint8_t (*onActive)(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor);
} BattleAction;

void BattleMenuWindow_update(RuntimeContext *ctx, TE_Img *screen, BattleMenuWindow* window, BattleMenu *battleMenu);
BattleMenuEntry BattleMenuEntry_fromAction(BattleAction *action);
void BattleState_updateActiveActions(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState);

#endif