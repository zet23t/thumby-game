#include "game_battle_actions.h"
#include "TE_math.h"


static uint8_t BattleAction_Thrust_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Thrust activated");
    return 1;
}

static uint8_t BattleAction_Thrust_OnActivating(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Thrust activating");
    return 1;
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
        .name = "Thrust",
        .actionPointCosts = 6,
        .onActivated = BattleAction_Thrust_OnActivated,
        .onActivating = BattleAction_Thrust_OnActivating,
        .onSelected = BattleAction_Thrust_OnSelected,
    };
}

static uint8_t BattleAction_Strike_OnActivated(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Strike activated");
    return 1;
}

static uint8_t BattleAction_Strike_OnActivating(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, BattleAction *action, BattleEntityState *actor)
{
    LOG("Strike activating");
    return 1;
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
        .name = "Strike",
        .actionPointCosts = 4,
        .onActivated = BattleAction_Strike_OnActivated,
        .onActivating = BattleAction_Strike_OnActivating,
        .onSelected = BattleAction_Strike_OnSelected,
    };
}

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
        return 1;
    }
    actor->actionTimer += ctx->deltaTime;
    progress = fTweenElasticOut(progress);
    BattleEntityState *nextTarget = &battleState->entities[actor->nextTarget];
    BattleEntityState *currentTarget = &battleState->entities[actor->target];
    BattlePosition *nextPosition = &battleState->positions[nextTarget->position];
    BattlePosition *currentPosition = &battleState->positions[currentTarget->position];
    int16_t x = (int16_t)(currentPosition->x + (nextPosition->x - currentPosition->x) * progress);
    int16_t y = (int16_t)(currentPosition->y + (nextPosition->y - currentPosition->y) * progress);
    playerCharacter.dirX = x - playerCharacter.x;
    playerCharacter.dirY = y - playerCharacter.y;
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

    return 0;
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

    BattleMenu battleMenu = {
        .selectedAction = action->selectedAction,
        .entries = entries,
        .entriesCount = entriesCount,
    };
    BattleMenuWindow_update(ctx, screen, &window, &battleMenu);
    action->selectedAction = battleMenu.selectedAction;
    if (ctx->inputA && !ctx->prevInputA)
    {
        int id = battleMenu.entries[battleMenu.selectedAction].id;
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