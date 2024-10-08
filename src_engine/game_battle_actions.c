#include "game_scenes.h"
#include "game_battle_actions.h"
#include "TE_math.h"

//# Overview
// Thrust action: Hits the target strongly, dealing 2 damage + 1 action point loss.
// Strike action: Hits the target, dealing 1 damage + 1 action point loss.
// Parry action: Blocks all next attacks.

//# Thrust action

static uint8_t BattleAction_Thrust_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    BattlePosition attackerPosition = battleState->positions[actor->position];
    BattleEntityState *target = &battleState->entities[actor->target];
    BattlePosition targetPosition = battleState->positions[target->position];
    float position = battleState->timer / 0.5f;
    float pingpong = 1.0f - fabsf(1.0f - position);
    float bumped = max_f(0.0f, pingpong * 10.0f - 9.0f);
    float dx = targetPosition.x - attackerPosition.x;
    float dy = targetPosition.y - attackerPosition.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f)
    {
        LOG("Invalid positionings / targets");
        return BATTLEACTION_ONACTIVATED_DONE;
    }
    float nx = dx / len;
    float ny = dy / len;
    float tx = targetPosition.x - nx * 6.0f;
    float ty = targetPosition.y - ny * 6.0f;
    // LOG("Thrust activated %.2f", mirrored);
    int16_t cx = attackerPosition.x + (tx - attackerPosition.x) * pingpong;
    int16_t cy = attackerPosition.y + (ty - attackerPosition.y) * pingpong;
    if (position >= 2.0f)
    {
        cx = attackerPosition.x;
        cy = attackerPosition.y;
    }

    Character *character = actor->id == 0 ? &playerCharacter : &enemies[actor->id - 1].character;

    float jump = sinf(pingpong * TE_PI * 0.85f);

    character->targetX = character->x = cx;
    character->targetY = character->y = cy - 10;
    character->flyHeight = max_f(0.0f, jump - .5f) * 15.0f;

    Character *targetCharacter = actor->target == 0 ? &playerCharacter : &enemies[actor->target - 1].character;
    targetCharacter->targetX = targetCharacter->x = targetPosition.x + nx * bumped * 3.0f;
    targetCharacter->targetY = targetCharacter->y = targetPosition.y + ny * bumped * 3.0f - 10;

    if (position >= 1.0f && (action->statusFlags & 1) == 0)
    {
        LOG("Thrust of %d hits %d", actor->id, actor->target);
        action->statusFlags |= 1;
        if (battleState->entities[actor->target].hitpoints > 2)
        {
            battleState->entities[actor->target].hitpoints -= 2;
        }
        else
        {
            battleState->entities[actor->target].hitpoints = 0;
        }
    }
    return position >= 2.0f ? BATTLEACTION_ONACTIVATED_DONE : BATTLEACTION_ONACTIVATED_CONTINUE;
}

static uint8_t BattleAction_Thrust_OnActivating(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Thrust activating");
    action->statusFlags = 0;
    return BATTLEACTION_ONACTIVATING_DONE;
}

static uint8_t BattleAction_Thrust_OnSelected(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    BattleEntityState *target = &battleState->entities[actor->target];
    BattlePosition *targetPosition = &battleState->positions[target->position];
    BattlePosition *actorPosition = &battleState->positions[actor->position];

    float dx = targetPosition->x - actorPosition->x;
    float dy = targetPosition->y - actorPosition->y;
    float distance = sqrtf(dx * dx + dy * dy);
    float nx = dx / distance;
    float ny = dy / distance;

    float angle = atan2f(dy, dx);
    float anim = fabsf(fmodf(ctx->time * 4.0f, 2.0f) - 1.0f);
    int16_t cx = (actorPosition->x + nx * (6.0f + anim * 4.0f));
    int16_t cy = (actorPosition->y + ny * (6.0f + anim * 4.0f));
    int16_t angleInt = (int16_t)(angle / TE_PI / 2.0f * 16.0f + 4.0f) & 15;
    uint8_t spriteIndex = SPRITE_FLAT_ARROW_2_0000 + angleInt;
    TE_Img_blitSprite(screen, GameAssets_getSprite(spriteIndex), cx, cy, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .tint = 1,
        .tintColor = DB32Colors[DB32_RED],
        .state = (TE_ImgOpState){
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = cy + 20,
            .zAlphaBlend = 1,
        }
    });

    return ctx->inputA && !ctx->prevInputA ? BATTLEACTION_ONSELECTED_ACTIVATE : BATTLEACTION_ONSELECTED_IGNORE;
}

BattleAction BattleAction_Thrust()
{
    return (BattleAction){
        .name = "Thrust: " 
            TX_SPRITE(SPRITE_HEART_HALF, 2, 2) TX_MOVECURSOR_X(253) 
            TX_SPRITE(SPRITE_HEART_HALF, 2, 2) TX_MOVECURSOR_X(2)
            TX_SPRITE(SPRITE_HOURGLASS_6, 2, 2),
        .actionPointCosts = 6,
        .onActivated = BattleAction_Thrust_OnActivated,
        .onActivating = BattleAction_Thrust_OnActivating,
        .onSelected = BattleAction_Thrust_OnSelected,
    };
}

//# Strike action

static uint8_t BattleAction_Strike_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Strike activated %.2f", battleState->timer);
    BattlePosition attackerPosition = battleState->positions[actor->position];
    BattleEntityState *target = &battleState->entities[actor->target];
    BattlePosition targetPosition = battleState->positions[target->position];
    float position = battleState->timer / 0.25f;
    float mirrored = 1.0f - fabsf(1.0f - position);
    float bumped = max_f(0.0f, mirrored * 10.0f - 9.0f);
    float dx = targetPosition.x - attackerPosition.x;
    float dy = targetPosition.y - attackerPosition.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f)
    {
        return BATTLEACTION_ONACTIVATED_DONE;
    }
    float nx = dx / len;
    float ny = dy / len;
    float tx = targetPosition.x - nx * 6.0f;
    float ty = targetPosition.y - ny * 6.0f;
    // LOG("Thrust activated %.2f", mirrored);
    int16_t cx = attackerPosition.x + (tx - attackerPosition.x) * mirrored;
    int16_t cy = attackerPosition.y + (ty - attackerPosition.y) * mirrored;
    if (position >= 2.0f)
    {
        cx = attackerPosition.x;
        cy = attackerPosition.y;
    }

    Character *character = actor->id == 0 ? &playerCharacter : &enemies[actor->id - 1].character;

    float jump = sinf(mirrored * TE_PI * 0.85f);

    character->targetX = character->x = cx;
    character->targetY = character->y = cy - 10;
    character->flyHeight = max_f(0.0f, jump - .5f) * 15.0f;

    Character *targetCharacter = actor->target == 0 ? &playerCharacter : &enemies[actor->target - 1].character;
    targetCharacter->targetX = targetCharacter->x = targetPosition.x + nx * bumped * 3.0f;
    targetCharacter->targetY = targetCharacter->y = targetPosition.y + ny * bumped * 3.0f - 10;

    
    if (position >= 1.0f && (action->statusFlags & 1) == 0)
    {
        LOG("Strike of %d hits %d", actor->id, actor->target);
        action->statusFlags |= 1;
        if (battleState->entities[actor->target].hitpoints > 1)
        {
            battleState->entities[actor->target].hitpoints -= 1;
        }
        else
        {
            battleState->entities[actor->target].hitpoints = 0;
        }
    }

    return position >= 2.0f ? BATTLEACTION_ONACTIVATED_DONE : BATTLEACTION_ONACTIVATED_CONTINUE;
}

static uint8_t BattleAction_Strike_OnActivating(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Strike activating");
    action->statusFlags = 0;
    return BATTLEACTION_ONACTIVATING_DONE;
}

static uint8_t BattleAction_Strike_OnSelected(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    BattleEntityState *target = &battleState->entities[actor->target];
    BattlePosition *targetPosition = &battleState->positions[target->position];
    BattlePosition *actorPosition = &battleState->positions[actor->position];

    float dx = targetPosition->x - actorPosition->x;
    float dy = targetPosition->y - actorPosition->y;
    float distance = sqrtf(dx * dx + dy * dy);
    float nx = dx / distance;
    float ny = dy / distance;

    float angle = atan2f(dy, dx);
    float anim = fabsf(fmodf(ctx->time * 4.0f, 2.0f) - 1.0f);
    int16_t cx = (actorPosition->x + nx * (6.0f + anim * 4.0f));
    int16_t cy = (actorPosition->y + ny * (6.0f + anim * 4.0f));
    int16_t angleInt = (int16_t)(angle / TE_PI / 2.0f * 16.0f + 4.0f) & 15;
    uint8_t spriteIndex = SPRITE_FLAT_ARROW_2_0000 + angleInt;
    TE_Img_blitSprite(screen, GameAssets_getSprite(spriteIndex), cx, cy, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .tint = 1,
        .tintColor = DB32Colors[DB32_RED],
        .state = (TE_ImgOpState){
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = cy + 20,
            .zAlphaBlend = 1,
        }
    });

    return ctx->inputA && !ctx->prevInputA ? BATTLEACTION_ONSELECTED_ACTIVATE : BATTLEACTION_ONSELECTED_IGNORE;
}

BattleAction BattleAction_Strike()
{
    return (BattleAction){
        .name = "Strike: " TX_SPRITE(SPRITE_HEART_HALF, 2, 2),
        .actionPointCosts = 4,
        .onActivated = BattleAction_Strike_OnActivated,
        .onActivating = BattleAction_Strike_OnActivating,
        .onSelected = BattleAction_Strike_OnSelected,
    };
}

//# Change target action

static uint8_t BattleAction_ChangeTarget_OnSelected(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    for (int i=0;i<battleState->entityCount;i++)
    {
        BattleEntityState *entity = &battleState->entities[i];
        if (entity->team != actor->team)
        {
            BattlePosition *position = &battleState->positions[entity->position];
            TE_Img_blitSprite(screen, GameAssets_getSprite(SPRITE_FLAT_ARROW_DOWN), position->x - 1, position->y - 14, (BlitEx)
            {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = DB32Colors[DB32_SKYBLUE],
                .state = (TE_ImgOpState){
                    .zCompareMode = Z_COMPARE_LESS,
                    .zValue = position->y + 20,
                    .zAlphaBlend = 1,
                }
            });
        }
    }
    return !ctx->prevInputA && ctx->inputA ? BATTLEACTION_ONSELECTED_ACTIVATE : BATTLEACTION_ONSELECTED_IGNORE;
}

static uint8_t BattleAction_ChangeTarget_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    float progress = actor->actionTimer / 1.0f;
    if (progress >= 1.0f)
    {
        actor->target = actor->nextTarget;
        return BATTLEACTION_ONACTIVATED_DONE;
    }
    actor->actionTimer += ctx->deltaTime;
    progress = fTweenElasticOut(progress);
    BattleEntityState *nextTarget = &battleState->entities[actor->nextTarget];
    BattleEntityState *currentTarget = &battleState->entities[actor->target];
    BattlePosition *nextPosition = &battleState->positions[nextTarget->position];
    BattlePosition *currentPosition = &battleState->positions[currentTarget->position];
    int16_t x = (int16_t)(currentPosition->x + (nextPosition->x - currentPosition->x) * progress);
    int16_t y = (int16_t)(currentPosition->y + (nextPosition->y - currentPosition->y) * progress);
    playerCharacter.dirX = sign_f(x - playerCharacter.x);
    playerCharacter.dirY = sign_f(y - playerCharacter.y);
    playerCharacter.dx = playerCharacter.dirX;
    playerCharacter.dy = playerCharacter.dirY;
    TE_Img_blitSprite(screen, GameAssets_getSprite(SPRITE_FLAT_ARROW_DOWN), x - 1, y - 14, (BlitEx)
    {
        .blendMode = TE_BLEND_ALPHAMASK,
        .tint = 1,
        .tintColor = DB32Colors[DB32_SKYBLUE],
        .state = (TE_ImgOpState){
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = y + 20,
            .zAlphaBlend = 1,
        }
    });

    return BATTLEACTION_ONACTIVATED_CONTINUE;
}

static uint8_t BattleAction_ChangeTarget_OnActivating (RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    for (int i=0;i<battleState->entityCount;i++)
    {
        BattleEntityState *entity = &battleState->entities[i];
        if (entity->team != actor->team)
        {
            // BattlePosition *position = &battleState->positions[entity->position];
            // if (ctx->inputA)
            // {
            //     actor->target = entity->position;
            //     return BATTLEACTION_ONACTIVATING_DONE;
            // }
        }
    }
    const int idCancel = 8;
    BattleMenuEntry entries[8];
    uint8_t entriesCount = 0;
    for (int i=0;i<battleState->entityCount;i++)
    {
        BattleEntityState *entity = &battleState->entities[i];
        if (entity->team != actor->team)
        {
            entries[entriesCount++] = BattleMenuEntryDef(entity->name,actor->target == i ? "0 AP" : "2 AP", i);

            if (entriesCount - 1 == action->selectedAction)
            {
                BattlePosition *position = &battleState->positions[entity->position];
                int16_t offset = (int)(fmodf(ctx->time * 2.0f, 1.0f) * 3);
                TE_Img_blitSprite(screen, GameAssets_getSprite(SPRITE_FLAT_ARROW_DOWN), position->x - 1, position->y - 14 + offset, (BlitEx)
                {
                    .blendMode = TE_BLEND_ALPHAMASK,
                    .tint = 1,
                    .tintColor = DB32Colors[DB32_SKYBLUE],
                    .state = (TE_ImgOpState){
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = position->y + 20,
                        .zAlphaBlend = 1,
                    }
                });
            }
        }
    }
    entries[entriesCount++] = BattleMenuEntryDef("Cancel", 0, idCancel);
    
    BattleMenuWindow window = {
        .x = -1,
        .y = -1,
        .w = 130,
        .h = 44,
        .divX = 80,
        .divX2 = 106,
        .lineHeight = 11,
        .selectedColor = 0x660099ff,
    };

    if (action->menu == 0)
    {
        action->menu = Scene_malloc(sizeof(BattleMenu));
        *action->menu = (BattleMenu) {
            .selectedAction = action->selectedAction,
        };
    }

    action->menu->entries = entries,
    action->menu->entriesCount = entriesCount,

    BattleMenuWindow_update(ctx, screen, &window, action->menu);
    action->selectedAction = action->menu->selectedAction;
    if (ctx->inputA && !ctx->prevInputA)
    {
        int id = action->menu->entries[action->menu->selectedAction].id;
        if (id == idCancel)
        {
            return BATTLEACTION_ONACTIVATING_CANCEL;
        }
        actor->actionTimer = 0.0f;
        actor->nextTarget = id;
        return BATTLEACTION_ONACTIVATING_DONE;
    }
    if (ctx->inputB)
    {
        return BATTLEACTION_ONACTIVATING_CANCEL;
    }
    return BATTLEACTION_ONACTIVATING_CONTINUE;
}

BattleAction BattleAction_ChangeTarget()
{
    return (BattleAction){
        .name = "Change Target",
        .actionPointCosts = 2,
        .onActivated = BattleAction_ChangeTarget_OnActivated,
        .onActivating = BattleAction_ChangeTarget_OnActivating,
        .onSelected = BattleAction_ChangeTarget_OnSelected,
    };
}

//# Parry action

typedef struct ParryData
{
    uint8_t lockedHealth;
} ParryData;

static uint8_t BattleAction_Parry_OnSelected(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    return ctx->inputA && !ctx->prevInputA ? BATTLEACTION_ONSELECTED_ACTIVATE : BATTLEACTION_ONSELECTED_IGNORE;
}

static uint8_t BattleAction_Parry_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    // LOG("Parry activated %.2f", action->actionTimer);
    float progress = action->actionTimer / 1.0f;
    action->actionTimer += ctx->deltaTime;
    if (progress >= 1.0f)
    {
        return BATTLEACTION_ONACTIVATED_ISACTIVE;
    }
    return BATTLEACTION_ONACTIVATED_CONTINUE;
}

static uint8_t BattleAction_Parry_OnActivating(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Parry activating");
    ParryData *data = (ParryData*)action->userData;
    data->lockedHealth = actor->hitpoints;
    action->actionTimer = 0;

    return BATTLEACTION_ONACTIVATING_DONE;
}

static uint8_t BattleAction_parry_OnActive(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    ParryData *data = (ParryData*)action->userData;
    if (data->lockedHealth > actor->hitpoints)
    {
        actor->hitpoints = data->lockedHealth;
    }
    BattlePosition *position = &battleState->positions[actor->position];
    TE_Img_blitSprite(screen, GameAssets_getSprite(SPRITE_SHIELD), position->x - 1, position->y - 14, (BlitEx)
    {
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = (TE_ImgOpState){
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = position->y + 20,
            .zAlphaBlend = 1,
        }
    });

    return actor->lastActionAtCounter + action->actionPointCosts > battleState->actionCounter ? BATTLEACTION_ONACTIVE_CONTINUE : BATTLEACTION_ONACTIVE_DONE;
}

BattleAction BattleAction_Parry()
{
    return (BattleAction){
        .name = "Parry: " TX_SPRITE(SPRITE_SHIELD, 2, 2),
        .actionPointCosts = 2,
        .userData = Scene_malloc(sizeof(ParryData)),
        .onActivated = BattleAction_Parry_OnActivated,
        .onActivating = BattleAction_Parry_OnActivating,
        .onSelected = BattleAction_Parry_OnSelected,
        .onActive = BattleAction_parry_OnActive,
    };
}

//# Insult action

static uint8_t BattleAction_Insult_OnSelected(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    return ctx->inputA && !ctx->prevInputA ? BATTLEACTION_ONSELECTED_ACTIVATE : BATTLEACTION_ONSELECTED_IGNORE;
}

static uint8_t BattleAction_Insult_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    return BATTLEACTION_ONACTIVATED_DONE;
}

static uint8_t BattleAction_Insult_OnActivating(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    const int idCancel = 8;
    BattleMenuEntry *entries = (BattleMenuEntry*)action->userData;
    uint8_t entriesCount = 0;
    for (entriesCount=0;entries[entriesCount].menuText;entriesCount++);
    
    BattleMenuWindow window = {
        .x = -1,
        .y = -1,
        .w = 130,
        .h = 44,
        .divX = 80,
        .divX2 = 106,
        .lineHeight = 11,
        .selectedColor = 0x660099ff,
    };

    if (action->menu == 0)
    {
        action->menu = Scene_malloc(sizeof(BattleMenu));
        *action->menu = (BattleMenu) {
            .selectedAction = action->selectedAction,
        };
    }

    action->menu->entries = entries;
    action->menu->entriesCount = entriesCount;
    BattleMenuWindow_update(ctx, screen, &window, action->menu);
    action->selectedAction = action->menu->selectedAction;
    if (ctx->inputA && !ctx->prevInputA)
    {
        int id = action->menu->entries[action->menu->selectedAction].id;
        if (id == idCancel)
        {
            return BATTLEACTION_ONACTIVATING_CANCEL;
        }
        actor->actionTimer = 0.0f;
        actor->nextTarget = id;
        return BATTLEACTION_ONACTIVATING_DONE;
    }
    if (ctx->inputB)
    {
        return BATTLEACTION_ONACTIVATING_CANCEL;
    }
    return BATTLEACTION_ONACTIVATING_CONTINUE;
}

BattleAction BattleAction_Insult(const char** insults)
{
    BattleMenuEntry *entries = Scene_malloc(sizeof(BattleMenuEntry) * 8);
    
    const int idCancel = 8;
    uint8_t entriesCount = 0;
    for (int i=0;insults[i];i++)
    {
        entries[entriesCount++] = BattleMenuEntryDef(insults[i], "2 AP", i);
    }
    entries[entriesCount++] = BattleMenuEntryDef("Cancel", 0, idCancel);

    return (BattleAction){
        .name = "Insult: " TX_SPRITE(SPRITE_HOURGLASS_6, 2, 2) TX_MOVECURSOR_X(253) TX_SPRITE(SPRITE_HOURGLASS_6, 2, 2),
        .userData = entries,
        .actionPointCosts = 1,
        .onActivated = BattleAction_Insult_OnActivated,
        .onActivating = BattleAction_Insult_OnActivating,
        .onSelected = BattleAction_Insult_OnSelected,
    };
}