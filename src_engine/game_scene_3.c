#include "game_scene_3.h"
#include "game_player.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_environment.h"
#include "game_enemies.h"
#include "game_assets.h"
#include "game_particlesystem.h"
#include "game.h"
#include "game_battle.h"
#include "game_battle_actions.h"

#include "TE_sdfmap.h"
#include "TE_math.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>

typedef struct Scene3_EnemyCrowd Scene3_EnemyCrowd;

typedef struct Scene3_EnemyData
{
    Scene3_EnemyCrowd *crowd;
    uint8_t flag;
    float koLocationX;
    float koLocationY;
} Scene3_EnemyData;

typedef struct Scene3_EnemyCrowd
{
    uint8_t aliveCount;
    uint8_t nextStepOnDefeated;
    int8_t selectedAttacker;
    Scene3_EnemyData enemies[4];
} Scene3_EnemyCrowd;

void Scene_3_enemyCallback(struct Enemy *enemy, EnemyCallbackArg arg, RuntimeContext *ctx, TE_Img *screen);

void Scene_3_envDebugDraw(RuntimeContext *ctx, TE_Img *screenData, ScriptedAction *action)
{
    // TE_SDFMap *sdfMap = (TE_SDFMap*)action->customCallback.dataPointer;
    // // if (ctx->inputB)
    // //     TE_SDFMap_drawDebug(sdfMap, ctx);
}
static TE_SDFMap *sdfMap = 0;


void BattleMenu_drawActionBars(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState, uint8_t selectedAPUse, BattleMenuWindow *window)
{
    // action bars
    int16_t x = window->x, y = window->y;
    int16_t w = window->w, h = window->h;
    int16_t divX2 = window->divX2;
    
    TE_Img_fillRect(screen, divX2, y + 1, w + x - divX2 - 1, h - 2, 0x88000000, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 200,
        .zAlphaBlend = 0,
    });
    uint32_t colors[5] = {0x33ff8888, 0x336688ff, 0x336688ff, 0x336688ff, 0x336688ff};
    // uint32_t targettedColors[5] = {0x33ff8888, 0x3366aaff, 0x3366aaff, 0x3366aaff, 0x3366aaff};
    const int16_t pxPerAP = 4;
    
    for (int i=0;i<battleState->entityCount;i++)
    {
        BattleEntityState *entity = &battleState->entities[i];
        if (entity->hitpoints <= 0)
        {
            continue;
        }
        int16_t x = divX2 + 1 + i * 4;
        uint32_t color = colors[i];
        for (int ap=1;ap * pxPerAP < h; ap++)
        {
            uint8_t alpha = (ap * 5);
            uint8_t r = min_s16(255, max_s16(((color) & 0xff) * 6 / (ap + 3) - alpha, 0));
            uint8_t g = min_s16(255, max_s16(((color >> 8) & 0xff) * 6 / (ap + 3) - alpha, 0));
            uint8_t b = min_s16(255, max_s16(((color >> 16) & 0xff) * 6 / (ap + 3) - alpha, 0));
            uint32_t rgba = (r) | (g << 8) | (b << 16) | 0xff000000;
            TE_Img_fillRect(screen, x, y + 3 + h - (ap + 1) * pxPerAP, 3, pxPerAP - 1, rgba, (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
                .zAlphaBlend = 0,
            });
        }
        // TE_Img_fillRect(screen, x, y + h - 8, 3, 6, 0x88000000, (TE_ImgOpState){
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        //     .zAlphaBlend = 1,
        // });

        if (entity->hitpoints > 0)
        {
            int apY = y + h - 3 - 2 - entity->actionPoints * pxPerAP;
            TE_Img_fillRect(screen, x, apY, 3, 3, 0xff000000, (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
                .zAlphaBlend = 1,
            });
            TE_Img_fillRect(screen, x, apY+1, 3, 1, 0xffffffff, (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
                .zAlphaBlend = 1,
            });

            if (selectedAPUse > 0 && i == 0 && fmodf(ctx->time * 2.0f, 1.0f) < 0.5f)
            {
                int futureAPY = apY - selectedAPUse * pxPerAP;
                TE_Img_fillRect(screen, x, futureAPY, 3, 3, 0xff000000, (TE_ImgOpState){
                    .zCompareMode = Z_COMPARE_ALWAYS,
                    .zValue = 200,
                    .zAlphaBlend = 1,
                });
                TE_Img_fillRect(screen, x, futureAPY+1, 3, 1, 0xffffffff, (TE_ImgOpState){
                    .zCompareMode = Z_COMPARE_ALWAYS,
                    .zValue = 200,
                    .zAlphaBlend = 1,
                });
            }
        }
    }
}

void Scene_3_battleStart(RuntimeContext *ctx, TE_Img *screen, ScriptedAction *action)
{
    Player_setInputEnabled(0);
    BattleState *battleState = (BattleState*)action->customCallback.dataPointer;
    battleState->timer += ctx->deltaTime;

    BattleState_updateActiveActions(ctx, screen, battleState);

    // update entities with enemies
    
    BattleEntityState *playerEntity = &battleState->entities[0];

    for (int i=0;i<battleState->entityCount;i++)
    {
        BattleEntityState *entity = &battleState->entities[i];
        BattlePosition *position = &battleState->positions[entity->position];
        int16_t targetX = position->x;
        int16_t targetY = position->y - 10;
        if (entity->id == 0)
        {
            playerEntity = entity;
            playerCharacter.targetX = targetX;
            playerCharacter.targetY = targetY;
            
            BattleEntityState *targetEntity = &battleState->entities[entity->target];
            BattlePosition* targetPosition = &battleState->positions[targetEntity->position];
            playerCharacter.dirX = sign_f(targetPosition->x - playerCharacter.x);
            playerCharacter.dirY = sign_f(targetPosition->y - playerCharacter.y);
        }
        else
        {
            Enemy* enemy = Enemies_getEnemy(entity->id, 1);
            if (enemy == 0 && entity->hitpoints > 0)
            {
                Enemies_spawn(entity->id, entity->characterType, targetX, targetY);
            }
            else if (enemy && entity->hitpoints <= 0)
            {
                enemy->health = 0.0f;
            }
            else if (enemy)
            {
                BattlePosition *position = &battleState->positions[entity->position];
                BattlePosition *targetPosition = &battleState->positions[entity->target];
                float dx = targetPosition->x - position->x;
                float dy = targetPosition->y - position->y;

                enemy->character.targetX = targetX;
                enemy->character.targetY = targetY;
                enemy->character.dirX = sign_f(dx);
                enemy->character.dirY = sign_f(dy);
            }
        }
    }

    // Handle running action
    if (battleState->queuedEntityId >= 0)
    {
        BattleEntityState *entity = &battleState->entities[battleState->queuedEntityId];
        BattleAction *action = &entity->actionNTList[battleState->queuedActionId];
        uint8_t result;
        if (battleState->queuedActionActivated == 0)
        {
            result = BATTLEACTION_ONACTIVATING_DONE;
            if (action->onActivating)
            {
                result = action->onActivating(ctx, screen, battleState, action, entity);
            }
            if (result == BATTLEACTION_ONACTIVATING_DONE)
            {
                battleState->queuedActionActivated = 1;
            }
        }
        else if (!action->onActivated || (result = action->onActivated(ctx, screen, battleState, action, entity)))
        {
            if (result == BATTLEACTION_ONACTIVATED_ISACTIVE && action->onActive)
            {
                LOG("Action %s (%d) is active; Entity %d at %d", action->name, action->actionPointCosts, entity->id, entity->actionPoints);
                battleState->activeActions[battleState->queuedEntityId] = action;
            }
            battleState->activatingAction = -1;
            battleState->queuedEntityId = -1;
            battleState->queuedActionId = -1;
            entity->actionPoints += action->actionPointCosts;
            entity->lastActionAtCounter = battleState->actionCounter;
            LOG("Action %s (%d) done; Entity %d at %d", action->name, action->actionPointCosts, entity->id, entity->actionPoints);
        }

        BattleMenu_drawActionBars(ctx, screen, battleState, 0, &battleState->menuWindow);
        
        return;
    }

    // decrease AP of all entities unless player has 1 AP; player actions can happen at AP 1 or AP 0. NPCs only at AP 0
    if (battleState->entities[0].actionPoints > 0)
    {
        if (battleState->timer > 0.25f)
        {
            uint8_t scheduledAction = 0;
            for (int i=0;i<battleState->entityCount;i++)
            {
                BattleEntityState *entity = &battleState->entities[i];
                // LOG("Entity %d/%d at %d", entity->id, entity->team, entity->actionPoints);
                if (entity->team == 1 && entity->actionPoints == 0 && entity->hitpoints > 0)
                {
                    // AI enemy turn
                    battleState->queuedEntityId = i;
                    battleState->queuedActionId = 0;
                    battleState->queuedActionActivated = 0;
                    battleState->timer = 0.0f;
                    scheduledAction = 1;

                    LOG("Scheduling action of enemy %d at %d", entity->id, entity->actionPoints);
                    break;
                }
            }
            if (!scheduledAction)
            {
                for (int i=0;i<battleState->entityCount;i++)
                {
                    BattleEntityState *entity = &battleState->entities[i];
                    if (entity->actionPoints > 0 && entity->hitpoints > 0)
                        entity->actionPoints -= 1;
                }
                battleState->actionCounter += 1;
                LOG("--> Action counter: %d", battleState->actionCounter);
            }
            battleState->timer = 0.0f;
        }
        BattleMenu_drawActionBars(ctx, screen, battleState, 0, &battleState->menuWindow);

        return;
    }

    

    int16_t x = -1, y = -1;
    int16_t w = 130, h = 44;

    int16_t divX = 80;
    int16_t divX2 = divX + 26;

    TE_Img_fillRect(screen, x, y, w, h, 0x88000000 | (0xffffff & DB32Colors[15]), (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 200,
        .zAlphaBlend = 1,
    });
    TE_Img_lineRect(screen, x, y, w, h, 0x33ffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 200,
        .zAlphaBlend = 1,
    });
    
    int8_t selectedAPUse = 0;
    if (battleState->activatingAction >= 0)
    {
        BattleAction *action = &playerEntity->actionNTList[battleState->activatingAction];
        selectedAPUse = action->actionPointCosts;
        if (action->onActivating)
        {
            uint8_t actionState = action->onActivating ? action->onActivating(ctx, screen, battleState, action, playerEntity) : BATTLEACTION_ONACTIVATING_CONTINUE;

            if (actionState == BATTLEACTION_ONACTIVATING_CANCEL || actionState == BATTLEACTION_ONACTIVATING_DONE)
            {
                battleState->activatingAction = -1;
            }
            if (actionState == BATTLEACTION_ONACTIVATING_DONE)
            {
                battleState->queuedActionId = battleState->selectedAction;
                battleState->queuedEntityId = 0;
                battleState->timer = 0.0f;
                battleState->queuedActionActivated = 1;
            }
        }
        else
        {
            battleState->activatingAction = -1;
            battleState->timer = 0.0f;
        }
    }
    else
    {
        BattleMenuWindow_update(ctx, screen, &battleState->menuWindow, &battleState->menu);
        battleState->selectedAction = battleState->menu.selectedAction;
        BattleAction *action = &playerEntity->actionNTList[battleState->selectedAction];
        selectedAPUse = action->actionPointCosts;

        if (action->onSelected && action->onSelected(ctx, screen, battleState, action, playerEntity) == BATTLEACTION_ONSELECTED_ACTIVATE)
        {
            battleState->activatingAction = battleState->selectedAction;
        }
    }

    // draw indicators which enemies will act after player
    uint8_t futureAP = playerEntity->actionPoints + selectedAPUse;
    for (int i=0;i<battleState->entityCount;i++)
    {
        BattleEntityState *entity = &battleState->entities[i];
        if (entity->hitpoints <= 0)
            continue;
        BattlePosition *position = &battleState->positions[entity->position];
        for (int j=0;j<entity->hitpoints;j++)
        {
            TE_Img_blitSprite(screen, GameAssets_getSprite(SPRITE_TINY_HEART), position->x - 4 + j * 3, position->y + 5, (BlitEx) {
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = position->y + 15,
                }
            });
        }
        if (entity->team == 1 && entity->actionPoints < futureAP)
        {
            float blink = fabsf(fmodf(ctx->time * 1.0f, 1.0f) - .5f) * 3.0f;
            uint8_t alpha = (uint8_t)fminf(blink * 255.0f, 255.0f);
            TE_Img_blitSprite(screen, GameAssets_getSprite(SPRITE_EXCLAMATION_MARK), position->x, position->y - 18, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = (DB32Colors[DB32_ORANGE] & 0xffffff) | (alpha << 24),
                .state = (TE_ImgOpState){
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = position->y + 18,
                    .zAlphaBlend = 1,
                }
            });
            // TE_Img_HLine(screen, position->x - 4, position->y + 3, 8, 0xff0000ff, (TE_ImgOpState){
            //     .zCompareMode = Z_COMPARE_LESS_EQUAL,
            //     .zValue = 200,
            //     .zAlphaBlend = 1,
            // });
        }
    }

    TE_Img_VLine(screen, divX + x, y + 1, h - 2, 0x33ffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 200,
        .zAlphaBlend = 1,
    });
    TE_Img_VLine(screen, divX2 + x, y + 1, h - 2, 0x33ffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 200,
        .zAlphaBlend = 1,
    });

    BattleMenu_drawActionBars(ctx, screen, battleState, selectedAPUse, &battleState->menuWindow);
    
    
}

uint8_t BattleState_getAliveTeamBits(BattleState *state)
{
    uint8_t aliveTeamBits = 0;
    for (int i=0;i<state->entityCount;i++)
    {
        BattleEntityState *entity = &state->entities[i];
        if (entity->hitpoints > 0)
        {
            aliveTeamBits |= 1 << entity->team;
        }
    }

    return aliveTeamBits;
}

uint8_t BattleState_haveNPCsWon(BattleState *state)
{
    return BattleState_getAliveTeamBits(state) == 2;
}

uint8_t BattleState_hasPlayerWon(BattleState *state)
{
    return BattleState_getAliveTeamBits(state) == 1;
}

static void Scene_3_subscene_1_init(uint8_t sceneId)
{
    player.x = 160;
    playerCharacter.x = player.x;
    player.y = 110;
    playerCharacter.y = player.y;

    uint8_t step = 0;

    ScriptedAction_addSetPlayerTarget(step, 0xff, 90, 73, 1, 1);
    ScriptedAction_addPlayerControlsEnabled(step, 0xff, 0);
    ScriptedAction_addSceneFadeOut(step, step, FADEIN_RIGHT_TO_LEFT, step + 1, 0.85f, 0.4f, 1.0f);
    step++;

    ScriptedAction_addSpeechBubble(step, step, "The shortcut over the stream. That way I can catch up!", 0, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addNPCSpawn(step, step + 50, 1, 3, -5, 60, 60, 70);
    ScriptedAction_addNPCSpawn(step, step + 50, 2, 4, -20, 60, 50, 66);
    ScriptedAction_addSpeechBubble(step, step, "Lucky we maintain the bridge so faithfully!", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Too bad it's so expensive to do that ...", 2, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Right, if only someone paid us to do so.", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Are you both born fools, or did you train for it?", 0, 8, 88, 112, 38, 0, 8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "My father built that bridge ...", 0, 8, 88, 112, 38, 0, 8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "... and it's the same pieces of wood from back then.", 0, 8, 88, 112, 38, 0, 8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Did he just insult us, Lenny?", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "I bet he did, Pip.", 2, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Normally it costs gold to cross the bridge.", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "But for you, we'll make an exception.", 2, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Thank goodness, that way I can catch them!", 0, 8, 88, 112, 38, 0, 8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "For you it'll cost gold to leave as well.", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Listen, the tax collector stole my cart full with gold!", 0, 8, 88, 112, 38, 0, 8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addAnimationPlayback(step,step + 10, ANIMATION_HAHAHA_RIGHT, 35,50,70, 0.0f, 1.0f, 1, DB32Colors[DB32_ORANGE]);
    ScriptedAction_addAnimationPlayback(step,step + 10, ANIMATION_HAHAHA_RIGHT, 30,55,70, 0.4f, 1.0f, 1, DB32Colors[DB32_ORANGE]);
    ScriptedAction_addAnimationPlayback(step,step + 10, ANIMATION_HAHAHA_LEFT, 47,55,70, 0.2f, 1.0f, 1, DB32Colors[DB32_BROWN]);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_WAIT, .wait.duration = 1.75f });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "The gold is always in the other pocket.", 2, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSetPlayerPosition(step, step, 90, 73, 1, 1);

    ScriptedAction_addSpeechBubble(step, step, "That's it, it's time to beat sense into you two!", 0, 8, 88, 112, 38, 0, 8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Ye think so? Too bad it's not just the two of us.", 2, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    
    ScriptedAction_addNPCSpawn(step, step + 50, 3, 3, 100, 0, 100, 40);
    ScriptedAction_addNPCSpawn(step, step + 50, 4, 4, 130, 120, 100, 96);
    ScriptedAction_addSetItem(step, step+2, 0, 0, ITEM_STAFF);
    ScriptedAction_addSetItem(step, step+2, 1, 0, ITEM_STAFF);
    ScriptedAction_addSetItem(step, step+2, 2, -ITEM_STAFF, 0);
    ScriptedAction_addSetItem(step, step+2, 3, 0, -ITEM_STAFF);
    ScriptedAction_addSetItem(step, step+2, 4, 0, ITEM_STAFF);

    ScriptedAction_addSpeechBubble(step, step, "Fine, let's have some fun first.", 0, 4, 4, 70, 48, -4, -8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_WAIT, .wait.duration = 2.5f });
    step++;
    
    ScriptedAction_addSpeechBubble(step, step, "Your name must be Sir Bigmouth.", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "You know what?", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Just you, and me. I'll teach you some manners.", 1, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Hahaha. That'll be too easy.", 0, 4, 4, 70, 48, -4, -8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addLoadScene(step, step, SCENE_4_FIRST_FIGHT);
}

static void Scene_3_subscene_2_init(uint8_t sceneId)
{
    uint8_t step = 0;

    player.x = 90;
    playerCharacter.x = player.x;
    player.y = 73;
    playerCharacter.y = player.y;

    // Scene3_EnemyCrowd *crowd = Scene_malloc(sizeof(Scene3_EnemyCrowd));
    // crowd->aliveCount = 4;
    // crowd->nextStepOnDefeated = step + 2;
    // crowd->enemies[0].crowd = crowd;
    // crowd->enemies[1].crowd = crowd;
    // crowd->enemies[2].crowd = crowd;
    // crowd->enemies[3].crowd = crowd;

    // ScriptedAction_addSetEnemyCallback(step, step, 1, (EnemyCallbackUserData) {
    //     .callback = Scene_3_enemyCallback,
    //     .dataPointer = &crowd->enemies[0],
    // });
    // ScriptedAction_addSetEnemyCallback(step, step, 2, (EnemyCallbackUserData) {
    //     .callback = Scene_3_enemyCallback,
    //     .dataPointer = &crowd->enemies[1],
    // });
    // ScriptedAction_addSetEnemyCallback(step, step, 3, (EnemyCallbackUserData) {
    //     .callback = Scene_3_enemyCallback,
    //     .dataPointer = &crowd->enemies[2],
    // });
    // ScriptedAction_addSetEnemyCallback(step, step, 4, (EnemyCallbackUserData) {
    //     .callback = Scene_3_enemyCallback,
    //     .dataPointer = &crowd->enemies[3],
    // });
    
    ScriptedAction_addNPCSpawn(step, step + 50, 1, 3, 60, 70, 60, 70);
    ScriptedAction_addNPCSpawn(step, step + 50, 2, 4, 50, 66, 50, 66);
    ScriptedAction_addNPCSpawn(step, step + 50, 3, 3, 100, 40, 100, 40);
    ScriptedAction_addNPCSpawn(step, step + 50, 4, 4, 100, 96, 100, 96);
    ScriptedAction_addPlayerControlsEnabled(step, step, 0);
    
    ScriptedAction_addJumpStep(step, step, step + 1);
    step++;

    BattleAction *playerActions = Scene_malloc(sizeof(BattleAction) * 8);
    BattleMenu *changeTargetMenu = Scene_malloc(sizeof(BattleMenu));
    changeTargetMenu->selectedAction = 0;

    playerActions[0] = BattleAction_Thrust();
    playerActions[1] = BattleAction_Strike();
    playerActions[2] = BattleAction_Parry();
    playerActions[3] = BattleAction_Insult((const char *[]){
        "A fine swing ... for a child!",
        "I've seen trees move faster than you.",
        "Aiming for the air, are we?",
        "Did you all practice missing together?",
        0
    });
    playerActions[4] = BattleAction_ChangeTarget();

    BattleAction *npcActions = Scene_malloc(sizeof(BattleAction) * 8);
    npcActions[0] = BattleAction_Thrust();

    BattleState *battleState = Scene_malloc(sizeof(BattleState));
    battleState->queuedEntityId = -1;
    battleState->activatingAction = -1;
    battleState->entityCount = 2;
    for (int i=0;i<5;i++)
    {
        battleState->entities[i].id = i;
        battleState->entities[i].team = i > 0 ? 1 : 0;
        battleState->entities[i].actionPoints = i == 0 ? 1 : 4 + i;
        battleState->entities[i].hitpoints = 3;
        battleState->entities[i].maxHitpoints = 3;
        battleState->entities[i].position = i;
        if (i > 0)
        {
            battleState->entities[i].actionNTList = npcActions;
        }
    }
    battleState->entities[0].hitpoints = 5;
    battleState->entities[0].maxHitpoints = 5;
    battleState->entities[0].target = 1;
    battleState->entities[0].actionNTList = playerActions;
    battleState->entities[1].name = "Lenny";
    battleState->entities[1].characterType = 3;
    battleState->entities[2].name = "Pip";
    battleState->entities[2].characterType = 4;
    battleState->entities[3].name = "Chuck";
    battleState->entities[3].characterType = 3;
    battleState->entities[4].name = "Mart";
    battleState->entities[4].characterType = 4;

    battleState->positions[0] = (BattlePosition){.x = 90, .y = 85};
    battleState->positions[1] = (BattlePosition){.x = 70, .y = 70};
    battleState->positions[2] = (BattlePosition){.x = 110, .y = 70};
    battleState->positions[3] = (BattlePosition){.x = 70, .y = 110};
    battleState->positions[4] = (BattlePosition){.x = 110, .y = 110};

    battleState->menuWindow.x = -1;
    battleState->menuWindow.y = -1;
    battleState->menuWindow.w = 130;
    battleState->menuWindow.h = 44;
    battleState->menuWindow.divX = 80;
    battleState->menuWindow.divX2 = 106;
    battleState->menuWindow.lineHeight = 11;
    battleState->menuWindow.selectedColor = 0x660099ff;

    BattleMenuEntry *mainEntries = Scene_malloc(sizeof(BattleMenuEntry) * 5);
    for (int i=0;i<5;i++)
    {
        mainEntries[i] = BattleMenuEntry_fromAction(&playerActions[i]);
    }

    battleState->menu = (BattleMenu){
        .selectedAction = 0,
        .entries = mainEntries,
        .entriesCount = 5,
    };

    ScriptedAction *battleAction = ScriptedAction_addCustomCallback(step, step + 1, Scene_3_battleStart);
    battleAction->customCallback.dataPointer = battleState;
    // fight
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ 
        .type = CONDITION_TYPE_CALLBACK_DATA,
        .callback.callbackRawData = (uint8_t(*)(void*)) BattleState_hasPlayerWon,
        .callback.callbackData = battleState
    });

    step++;
    LOG("final Step: %d", step);
    ScriptedAction_addSpeechBubble(step, step, "Aaaand I am done here.", 0, 4, 4, 70, 48, -4, -8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_WAIT, .wait.duration = 2.5f });
    step++;
    ScriptedAction_addLoadScene(step, step, SCENE_PLAYED_THROUGH);
}

void Scene_3_init(uint8_t sceneId)
{
    sdfMap = 0;
    Environment_addTreeGroup(20, 20, 18522, 4, 20);
    Environment_addTree(88,20,124);
    Environment_addTree(114,34,125);
    Environment_addTree(108,14,125);
    Environment_addTree(120,20,125);
    Environment_addTree(94,20,125);
    // Environment_addTreeGroup(100, 20, 1856, 5, 30);
    Environment_addFlowerGroup(76,40, 232, 15, 20);
    Environment_addBushGroup(75,35, 1232, 5, 10);


    switch (sceneId)
    {
        case SCENE_3_CHASING_THE_LOOT:
            Scene_3_subscene_1_init(sceneId);
            break;
        case SCENE_4_FIRST_FIGHT:
            Scene_3_subscene_2_init(sceneId);
            break;
    }

}

void Scene_3_drawKOEnemy(RuntimeContext *ctx, TE_Img *screenData, ScriptedAction *action)
{
    Enemy *enemy = action->customCallback.enemyPointer;
    float t = ctx->time - action->actionStartTime;
    float x = action->customCallback.x;
    float y = action->customCallback.y;

    const float duration = 0.5f;
    float progress = t / duration;
    float zOffset = 0;
    if (progress < 1.0f)
    {
        x = fLerp(x, enemy->character.targetX, progress);
        y = fLerp(y, enemy->character.targetY, progress);
        float fly = sinf(progress * 3.1415f);
        y += -fly * 5.0f + progress * 5.0f;
        zOffset = fly * 3.0f;
    }
    else
    {
        x = enemy->character.targetX;
        y = enemy->character.targetY;
        y += 5.0f;
        if (action->customCallback.flag == 0)
        {
            action->customCallback.flag = 1;
            for (int i=0;i<10;i++)
            {
                float vx = enemy->character.targetX - action->customCallback.x;
                float vy = enemy->character.targetY - action->customCallback.y;
                vx *= 2.0f;
                vy *= 2.0f;
                vx+= TE_randRange(-30, 30);
                vy+= TE_randRange(-30, 30);
                int16_t px = TE_randRange(x - 5, x + 5);
                int16_t py = TE_randRange(y +4, y + 8);
                for (int j=0;j<4;j++)
                    ParticleSystem_spawn(PARTICLE_TYPE_SIMPLE, px + j/2, py +  j%2, py + 20, 
                        vx + j/2*6-3, vy + j%2*6-3, (ParticleTypeData){
                        .simpleType = {
                            .color = DB32Colors[DB32_TAN],
                            .maxLife = TE_randRange(100,200)*0.01f,
                            .accelY = -22.5f,
                            .accelX = 0.0f,
                            .drag = 4.5f,
                            .size = 0,
                        },
                    });
            }
        }
    }
    
    enemy->character.x = x;
    enemy->character.y = y;
    Character_drawKO(screenData, &enemy->character, zOffset);
}

static void Scene_3_enemyTookDamage(struct Enemy *enemy, float damage, float vx, float vy, RuntimeContext *ctx, TE_Img *screen)
{
    Scene3_EnemyData *data = (Scene3_EnemyData*)enemy->userCallbackData.dataPointer;
    LOG("Callback: Enemy took damage %f, aliveCount=%d, selectedAttacker=%d, %p", damage, data->crowd->aliveCount, data->crowd->selectedAttacker, Enemies_getEnemy(data->crowd->selectedAttacker, 0));
    int seekIncrement = TE_randRange(1, 11);
    while (data->crowd->aliveCount > 1 && (data->crowd->selectedAttacker == enemy->id ||
        !Enemies_getEnemy(data->crowd->selectedAttacker, 0)))
    {
        player.defenseActionStep[0] = 0.0f;
        data->crowd->selectedAttacker = (data->crowd->selectedAttacker + seekIncrement - 1) % 4 + 1;
        if (seekIncrement > 1)
        {
            seekIncrement--;
        }
        LOG("Selected attacker %d took damage, selecting new: %d", enemy->id, data->crowd->selectedAttacker);
    }

    if (enemy->health <= 0.0f)
    {
        data->crowd->aliveCount--;
        LOG("Enemy eliminated, remaining: %d", data->crowd->aliveCount);
        if (data->crowd->aliveCount == 0)
        {
            LOG("All enemies eliminated, proceeding to %d", data->crowd->nextStepOnDefeated);
            Scene_setStep(data->crowd->nextStepOnDefeated);
        }
        ScriptedAction* action = ScriptedAction_addCustomCallback(0, 0xff, Scene_3_drawKOEnemy);
        action->customCallback.dataPointer = enemy;
        action->customCallback.x = enemy->character.x;
        action->customCallback.y = enemy->character.y;
        enemy->character.targetX = enemy->character.x + vx * 0.05f;
        enemy->character.targetY = enemy->character.y + vy * 0.05f;
        action->customCallback.flag = 0;
        action->actionStartTime = ctx->time;
    }
}

static void Scene_3_updateEnemy(struct Enemy *enemy, RuntimeContext *ctx, TE_Img *screen)
{
    Scene3_EnemyData *data = (Scene3_EnemyData*)enemy->userCallbackData.dataPointer;
    float playerDX = playerCharacter.x - enemy->character.x;
    float playerDY = playerCharacter.y - enemy->character.y;
    float playerDistance = sqrtf(playerDX * playerDX + playerDY * playerDY);

    if (enemy->id == 2 && data->crowd->aliveCount > 1) {
        // guard
        enemy->character.targetX = 50;
        enemy->character.targetY = 70;
        if (data->crowd->selectedAttacker == enemy->id)
        {
            data->crowd->selectedAttacker = 0;
        }
    }
    else if (playerDistance > 0.0f)
    {
        float chosenDistance = 40.0f;
        if (data->crowd->selectedAttacker == 0)
        {
            data->crowd->selectedAttacker = enemy->id;
        }
        enemy->character.dirX = playerDX > 0 ? 1 : -1;
        enemy->character.dirY = playerDY > 0 ? 1 : -1;
        if (data->crowd->selectedAttacker == enemy->id)
        {
            chosenDistance = 15.0f;
            enemy->character.maskDir = 1;
            if (playerDistance < 18.0f)
            {
                player.defenseActionStep[0]+=ctx->deltaTime;
                if (!ctx->inputA && ctx->prevInputA && player.defenseQuality > 0 && !Menu_isActive())
                {
                    data->crowd->selectedAttacker = 0;
                    enemy->character.isAiming = 0;
                    enemy->character.isStriking = 0;
                    enemy->character.runningAnimationTime = 0.0f;
                    player.defenseActionStep[0] = 0.0f;
                    return;
                }
                if (player.defenseActionStep[0] > 1.75f)
                {
                    if (enemy->character.isAiming)
                    {
                        enemy->character.isAiming = 0;
                        enemy->character.isStriking = 1;
                        enemy->character.runningAnimationTime = 0.0f;
                    }
                    if (!enemy->character.isStriking)
                    {
                        // strike is done
                        data->crowd->selectedAttacker = 0;
                    }
                }
                else if (player.defenseActionStep[0] > 0.25f)
                {
                    enemy->character.isAiming = 1;
                }
            }
            else
            {
                player.defenseActionStep[0] = 0.0f;
                enemy->character.isAiming = 0;
            }
        }
        else
        {
            enemy->character.maskDir = enemy->character.targetDistance < 3.0f;
        }

        for (int i = 0; i < 8; i++)
        {
            float nx = playerDX / playerDistance;
            float ny = playerDY / playerDistance;
            enemy->character.targetX = playerCharacter.x - nx * chosenDistance;
            enemy->character.targetY = playerCharacter.y - ny * chosenDistance;
            int16_t cx, cy;
            float cdist = Environment_calcSDFValue(enemy->character.targetX, enemy->character.targetY, &cx, &cy);
            if (enemy->character.targetX > 120.0f || enemy->character.targetX < 8.0f || enemy->character.targetY > 120.0f || enemy->character.targetY < 8.0f)
            {
                cdist = 0;
            }
            if (cdist > 5.0f || i == 7)
            {
                break;
            }
            // Let each NPC move in another direction & hope for the best
            float avoidNX = enemy->id & 1 ? -ny : ny;
            float avoidNY = enemy->id & 1 ? nx : -nx;
            
            enemy->character.targetX += avoidNX * 8.0f;
            enemy->character.targetY += avoidNY * 8.0f;
            // update player distance & direction
            playerDX = playerCharacter.x - enemy->character.targetX;
            playerDY = playerCharacter.y - enemy->character.targetY;
            playerDistance = sqrtf(playerDX * playerDX + playerDY * playerDY);
        }
    }
}

void Scene_3_enemyCallback(struct Enemy *enemy, EnemyCallbackArg arg, RuntimeContext *ctx, TE_Img *screen)
{
    switch (arg.type) {
        case ENEMY_CALLBACK_TYPE_TOOK_DAMAGE: Scene_3_enemyTookDamage(enemy, arg.tookDamage.damage, arg.tookDamage.vx, arg.tookDamage.vy, ctx, screen); return;
        case ENEMY_CALLBACK_TYPE_UPDATE: Scene_3_updateEnemy(enemy, ctx, screen); return;
    }
}

void Scene_3_update(RuntimeContext *ctx, TE_Img *screenData)
{
    uint8_t initSDF = sdfMap == 0;
    if (initSDF)
    {
        sdfMap = (TE_SDFMap*) Scene_malloc(sizeof(TE_SDFMap));
        *sdfMap = (TE_SDFMap) {
            .data = (TE_SDFCell*) Scene_malloc(128 * 128 * sizeof(TE_SDFCell)),
            .width = 128,
            .height = 128,
        };
        Environment_updateSDFMap(sdfMap);
        Environment_setSDFMap(sdfMap);
        ScriptedAction_addCustomCallback(0, 0xff, Scene_3_envDebugDraw)->customCallback.dataPointer = sdfMap;
    }

    // river
    int16_t flowOffset1 = (int16_t)(ctx->time * 11.0f);
    int16_t flowOffset2 = (int16_t)(ctx->time * 14.5f);
    int16_t flowOffsetAcc1 = 0;
    int16_t flowOffsetAcc2 = 0;
    TE_randSetSeed(32912);
    for (int y=0;y<128;y++)
    {
        int16_t x = (int16_t)(sin(y * 0.1f) * 5.0f + cos(y * 0.035f) * 8.0f + 44 - y * 0.3f);
        int16_t width = (int16_t)(sin(y * 0.06f) * 5.0f + 5); 
        flowOffsetAcc1 += width;
        flowOffsetAcc2 += width + x - 44;

        if (TE_randRange(0, 100) < 25)
        {
            // little rocks
            TE_Img_blitEx(screenData, &atlasImg, x + TE_randRange(-width / 2 - 10, -width / 2 + 3), y, 
                144 + TE_rand() % 4 * 8, 32 + TE_rand() % 2 * 8, 8, 8, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = 2,
                }
            });
            TE_Img_blitEx(screenData, &atlasImg, x + TE_randRange(width / 2 +20, +width / 2 + 30), y, 
                144 + TE_rand() % 4 * 8, 32 + TE_rand() % 2 * 8, 8, 8, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = 2,
                }
            });
        }
        
        TE_Img_blitEx(screenData, &atlasImg, x - width / 2-5, y, 176, y%48, 10, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x + width / 2 + 27, y, 176, (48*48-y + 17)%48, 10, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .flipX = 1,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });

        if (initSDF) TE_SDFMap_setRect(sdfMap, x - width / 2 - 2, y, width + 38, 1, 1);

        // flowing water
        TE_Img_blitEx(screenData, &atlasImg, x - width / 2, y, 112, y%32, 16, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 1,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x - width / 2 + width + 16, y, 128, y%32, 16, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 1,
            }
        });
        TE_Img_fillRect(screenData, x + 16 - width / 2, y, width, 1, DB32Colors[17], (TE_ImgOpState){
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = 1,
        }); 

        TE_Img_blitEx(screenData, &atlasImg, x + 1, y, 144, (y - flowOffset1 + flowOffsetAcc1 / 14)&31, 32, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 1,
            }
        });

        TE_Img_blitEx(screenData, &atlasImg, x - 1, y, 144, (y - flowOffset2 + flowOffsetAcc2 / 74)&31, 32, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .flipX = 1,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 1,
            }
        });
        
    }

    
    // path to bridge from right side
    for (int x=52;x<128;x+=2)
    {
        int16_t y = 63+ (int16_t)(sin((x * 1.2f+12.0f) * 0.05f) * 5.0f + x * 0.2f);
        TE_Img_blitEx(screenData, &atlasImg, x, y, 104 + (x%24), 32, 2, 14, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
    }
    // path to bridge from left side
    for (int x=0;x<14;x+=2)
    {
        int16_t y = 63+ (int16_t)(sin((x * 1.2f+10.0f) * 0.05f) * 5.0f - x * 0.2f);
        TE_Img_blitEx(screenData, &atlasImg, x, y, 104 + (x%24), 32, 2, 14, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
    }

    // bridge drawing
    TE_randSetSeed(ctx->frameCount/4);
    const int bridgeXEnd = 54;
    for (int x = 14, p=0; x < bridgeXEnd; x++,p++)
    {
        int y = 64 + x / 8;
        int xoffset = p%16;
        if (p < 3)
        {
            xoffset -= 1;
        }
        else if (bridgeXEnd - x < 4)
        {
            xoffset = 17 - (bridgeXEnd - x);
        }
        TE_Img_blitEx(screenData, &atlasImg, x, y, 144 + xoffset, 48, 1, 16, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 3,
            }
        });
        if (initSDF)
        {
            TE_SDFMap_setRect(sdfMap, x, y, 3, 12, 0);
        }
        int bridgeHeightLeft = p - 5;
        int bridgeHeightRight = bridgeXEnd - x - 7;
        int bridgeHeight = bridgeHeightLeft < bridgeHeightRight ? bridgeHeightLeft : bridgeHeightRight;
        if (bridgeHeight > 0)
        {
            if (bridgeHeight > 5) bridgeHeight = 5;
            bridgeHeight += TE_rand() % 2;
            int polePos = p%13;
            if (polePos >= 3 && polePos <= 6) bridgeHeight += 2;
            // bridge shadow
            TE_Img_fillRect(screenData, x, y + 14, 1, bridgeHeight, DB32Colors[16], (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 1,
            });
        }
        // poles
        if (p % 13 == 5)
        {
            TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_POLE_TOP), x + 2, y, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = 2,
                }
            });
            TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_POLE_TOP), x, y + 13, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = y + 23,
                }
            });
        }
        // guardrail
        TE_Img_blitEx(screenData, &atlasImg, x, y-6, 162 + xoffset, 56, 1, 4, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = y + 10,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x, y+8, 162 + xoffset, 56, 1, 4, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = y + 25,
            }
        });
    }

    if (initSDF) TE_SDFMap_compute(sdfMap);
}