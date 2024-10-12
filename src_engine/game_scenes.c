#include "game_scenes.h"
#include "game_enemies.h"
#include "game_player.h"
#include "game_assets.h"
#include "game_environment.h"
#include "game_particlesystem.h"
#include "game_renderobjects.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>
#include <memory.h>

int8_t _loadNextScene = -1;

void DrawNextButtonAction(RuntimeContext *ctx, TE_Img *screenData)
{
    int16_t pressY = (int16_t)(fmodf(ctx->time, 0.6f) * 2.0f) + 117;
    TE_Img_fillCircle(screenData, 117, pressY, 6, DB32Colors[18], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Img_lineCircle(screenData, 117, pressY, 6, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Img_lineCircle(screenData, 117, pressY+1, 6, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS,
        .zValue = 255,
    });
    TE_Img_lineCircle(screenData, 117, 119, 6, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS,
        .zValue = 255,
    });
    TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_ARROW_RIGHT), 118, pressY, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_ALWAYS,
            .zValue = 255,
        }
    });
}

uint8_t Condition_update(const Condition *condition, RuntimeContext *ctx, TE_Img *screenData, float time)
{
    if (condition->type == CONDITION_TYPE_PLAYER_IN_RECT)
    {
        int16_t x = playerCharacter.x, y = playerCharacter.y;
        // TE_Logf("CONDITION", "Player pos: %d %d -> %d %d, rect: %d %d %d %d", x, y, 
        //     (int)playerCharacter.targetX, (int)playerCharacter.targetY,
        //     condition->x, condition->y, 
        //     condition->width, condition->height);
        return (x >= condition->npcsInRect.x && x < condition->npcsInRect.x + condition->npcsInRect.width
            && y >= condition->npcsInRect.y && y < condition->npcsInRect.y + condition->npcsInRect.height);
    }

    if (condition->type == CONDITION_TYPE_NPCS_IN_RECT)
    {
        for (int i=0;i<4;i++)
        {
            float x, y;
            if (Enemies_getPosition(condition->npcsInRect.npcIds[i], &x, &y))
            {
                if (x < condition->npcsInRect.x || x > condition->npcsInRect.x + condition->npcsInRect.width
                    || y < condition->npcsInRect.y || y > condition->npcsInRect.y + condition->npcsInRect.height)
                {
                    return 0;
                }
            }
        }
        return 1;
    }

    if (condition->type == CONDITION_TYPE_WAIT)
    {
        return time >= condition->wait.duration;
    }

    if (condition->type == CONDITION_TYPE_PRESS_NEXT)
    {
        DrawNextButtonAction(ctx, screenData);
        return ctx->inputRight && !ctx->prevInputRight;
    }

    return 0;
}


struct ScriptedActions scriptedActions;

#define SCRIPTED_ACTION_TYPE_NONE 0
#define SCRIPTED_ACTION_TYPE_SPEECH_BUBBLE 1
#define SCRIPTED_ACTION_TYPE_SET_PLAYER_CONTROLS_ENABLED 2
#define SCRIPTED_ACTION_TYPE_PROCEED_PLOT_CONDITION 3
#define SCRIPTED_ACTION_TYPE_SET_NPC_TARGET 4
#define SCRIPTED_ACTION_TYPE_SET_FLAGS 5
#define SCRIPTED_ACTION_TYPE_SET_PLAYER_TARGET 6
#define SCRIPTED_ACTION_TYPE_SET_NPC_HEALTH 7
#define SCRIPTED_ACTION_TYPE_SCENE_FADE_OUT 8
#define SCRIPTED_ACTION_TYPE_TITLE_SCREEN 9
#define SCRIPTED_ACTION_TYPE_LOAD_SCENE 10
#define SCRIPTED_ACTION_TYPE_CLEAR_SCREEN 11
#define SCRIPTED_ACTION_TYPE_NPC_SPAWN 12
#define SCRIPTED_ACTION_TYPE_SET_ITEM 13
#define SCRIPTED_ACTION_TYPE_ANIMATION_PLAYBACK 14
#define SCRIPTED_ACTION_TYPE_SET_PLAYER_POSITION 15
#define SCRIPTED_ACTION_TYPE_SET_ENEMY_CALLBACK 16
#define SCRIPTED_ACTION_TYPE_CUSTOM_CALLBACK 17

void ScriptedAction_init()
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_NONE;
        scriptedActions.actions[i].actionStartTime = 0.0f;
    }
    scriptedActions.currentPlotIndex = 0;
    scriptedActions.flags = 0;
    scriptedActions.startedTimerPlotIndex = 0xff;
    scriptedActions.plotIndexStartTime = 0.0f;
}

ScriptedAction* ScriptedAction_addSpeechBubble(uint8_t stepStart, uint8_t stepStop, const char *text, uint8_t speaker, int16_t speechBubbleX, int16_t speechBubbleY, uint8_t speechBubbleWidth, uint8_t speechBubbleHeight, int8_t arrowXOffset, int8_t arrowYOffset)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SPEECH_BUBBLE;
            scriptedActions.actions[i].speechBubble.text = text;
            scriptedActions.actions[i].speechBubble.speaker = speaker;
            scriptedActions.actions[i].speechBubble.speechBubbleX = speechBubbleX;
            scriptedActions.actions[i].speechBubble.speechBubbleY = speechBubbleY;
            scriptedActions.actions[i].speechBubble.speechBubbleWidth = speechBubbleWidth;
            scriptedActions.actions[i].speechBubble.speechBubbleHeight = speechBubbleHeight;
            scriptedActions.actions[i].speechBubble.arrowXOffset = arrowXOffset;
            scriptedActions.actions[i].speechBubble.arrowYOffset = arrowYOffset;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return &scriptedActions.actions[i];
        }
    }

    return NULL;
}

void ScriptedAction_addPlayerControlsEnabled(uint8_t stepStart, uint8_t stepStop, uint8_t enabled)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_PLAYER_CONTROLS_ENABLED;
            scriptedActions.actions[i].playerControlsData.enabled = enabled;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addSetPlayerTarget(uint8_t stepStart, uint8_t stepStop, int16_t x, int16_t y, uint8_t setX, uint8_t setY)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_PLAYER_TARGET;
            scriptedActions.actions[i].playerTarget.targetX = x;
            scriptedActions.actions[i].playerTarget.targetY = y;
            scriptedActions.actions[i].playerTarget.setX = setX;
            scriptedActions.actions[i].playerTarget.setY = setY;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}
void ScriptedAction_addSetPlayerPosition(uint8_t stepStart, uint8_t stepStop, int16_t x, int16_t y, uint8_t setX, uint8_t setY)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_PLAYER_POSITION;
            scriptedActions.actions[i].playerTarget.targetX = x;
            scriptedActions.actions[i].playerTarget.targetY = y;
            scriptedActions.actions[i].playerTarget.setX = setX;
            scriptedActions.actions[i].playerTarget.setY = setY;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}
void ScriptedAction_addSetNPCTarget(uint8_t stepStart, uint8_t stepStop, uint8_t id, int16_t x, int16_t y)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_NPC_TARGET;
            scriptedActions.actions[i].npcTarget.id = id;
            scriptedActions.actions[i].npcTarget.x = x;
            scriptedActions.actions[i].npcTarget.y = y;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addProceedPlotCondition(uint8_t stepStart, uint8_t stepStop, uint8_t setPlotIndex, Condition condition)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_PROCEED_PLOT_CONDITION;
            scriptedActions.actions[i].proceedPlotCondition.setPlotIndex = setPlotIndex;
            scriptedActions.actions[i].proceedPlotCondition.condition = condition;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addSetFlags(uint8_t stepStart, uint8_t stepStop, uint32_t setFlags, uint32_t mask)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_FLAGS;
            scriptedActions.actions[i].setFlags.setFlags = setFlags;
            scriptedActions.actions[i].setFlags.mask = mask;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addSetNPCHealth(uint8_t stepStart, uint8_t stepStop, uint8_t id, float health)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_NPC_HEALTH;
            scriptedActions.actions[i].npcHealth.id = id;
            scriptedActions.actions[i].npcHealth.health = health;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addJumpStep(uint8_t stepStart, uint8_t stepStop, uint8_t stepTo)
{
    Condition condition = {
        .type = CONDITION_TYPE_WAIT,
        .wait = {
            .duration = 0.0f,
        }
    };
    ScriptedAction_addProceedPlotCondition(stepStart, stepStop, stepTo, condition);
}

void ScriptedAction_addSetEnemyCallback(uint8_t stepStart, uint8_t stepStop, uint8_t id, EnemyCallbackUserData callback)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_ENEMY_CALLBACK;
            scriptedActions.actions[i].setEnemyCallback.id = id;
            scriptedActions.actions[i].setEnemyCallback.callback = callback;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addSetItem(uint8_t stepStart, uint8_t stepStop, uint8_t charId, int8_t leftItemIndex, int8_t rightItemIndex)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_ITEM;
            scriptedActions.actions[i].setItem.charId = charId;
            scriptedActions.actions[i].setItem.leftItemIndex = leftItemIndex;
            scriptedActions.actions[i].setItem.rightItemIndex = rightItemIndex;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}


void ScriptedAction_addSceneFadeOut(uint8_t stepStart, uint8_t stepStop, uint8_t type, uint8_t nextPlotIndex, float duration, float beginDelay, float finishDelay)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SCENE_FADE_OUT;
            scriptedActions.actions[i].sceneFadeOut.type = type;
            scriptedActions.actions[i].sceneFadeOut.duration = duration;
            scriptedActions.actions[i].sceneFadeOut.beginDelay = beginDelay;
            scriptedActions.actions[i].sceneFadeOut.finishDelay = finishDelay;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            scriptedActions.actions[i].sceneFadeOut.nextPlotIndex = nextPlotIndex;
            return;
        }
    }
}

void ScriptedAction_addTitleScreen(uint8_t stepStart, uint8_t stepStop, const char *text, const char *subtitle, uint8_t fillBlackBackground, uint8_t nextPlotIndex)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_TITLE_SCREEN;
            scriptedActions.actions[i].titleScreen.titleText = text;
            scriptedActions.actions[i].titleScreen.subText = subtitle;
            scriptedActions.actions[i].titleScreen.fillBlackBackground = fillBlackBackground;
            scriptedActions.actions[i].titleScreen.nextPlotIndex = nextPlotIndex;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addLoadScene(uint8_t stepStart, uint8_t stepStop, uint8_t sceneId)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_LOAD_SCENE;
            scriptedActions.actions[i].loadScene.sceneId = sceneId;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addClearScreen(uint8_t stepStart, uint8_t stepStop, uint32_t color, uint8_t z)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_CLEAR_SCREEN;
            scriptedActions.actions[i].clearScreen.color = color;
            scriptedActions.actions[i].clearScreen.z = z;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addNPCSpawn(uint8_t stepStart, uint8_t stepStop, uint8_t npcId, uint8_t characterType, 
    int16_t x, int16_t y, int16_t targetX, int16_t targetY)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        ScriptedAction *action = &scriptedActions.actions[i];
        if (action->actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            action->actionType = SCRIPTED_ACTION_TYPE_NPC_SPAWN;
            action->npcSpawn.id = npcId;
            action->npcSpawn.characterType = characterType;
            action->npcSpawn.x = x;
            action->npcSpawn.y = y;
            action->npcSpawn.targetX = targetX;
            action->npcSpawn.targetY = targetY;
            action->startPlotIndex = stepStart;
            action->endPlotIndex = stepStop;
            return;
        }
    }
}

ScriptedAction* ScriptedAction_addCustomCallback(uint8_t stepStart, uint8_t stepStop, void(*callback)(RuntimeContext *ctx, TE_Img *screenData, ScriptedAction *callbackData))
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        ScriptedAction *action = &scriptedActions.actions[i];
        if (action->actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            action->actionType = SCRIPTED_ACTION_TYPE_CUSTOM_CALLBACK;
            action->customCallback.callback = callback;
            action->startPlotIndex = stepStart;
            action->endPlotIndex = stepStop;
            return action;
        }
    }
    return NULL;
}

void ScriptedAction_addAnimationPlayback(uint8_t stepStart, uint8_t stepStop, uint8_t animationId, int16_t x, int16_t y, uint8_t z,
    float delay, float speed, uint8_t loop, uint32_t tintColor)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        ScriptedAction *action = &scriptedActions.actions[i];
        if (action->actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            action->actionType = SCRIPTED_ACTION_TYPE_ANIMATION_PLAYBACK;
            action->animationPlayback.animationId = animationId;
            action->animationPlayback.x = x;
            action->animationPlayback.y = y;
            action->animationPlayback.z = z;
            action->animationPlayback.delay = delay;
            action->animationPlayback.speed = speed;
            action->animationPlayback.maxLoopCount = loop;
            action->startPlotIndex = stepStart;
            action->endPlotIndex = stepStop;
            action->animationPlayback.tintColor = tintColor;
            return;
        }
    }
}

void ScriptedAction_update(RuntimeContext *ctx, TE_Img *screenData)
{
    uint32_t oldSeed = TE_randSetSeed(ctx->frameCount * 13 + 8992);
    uint8_t nextPlotIndex = scriptedActions.currentPlotIndex;
    uint32_t nextFlags = scriptedActions.flags;
    uint8_t isNewStep;
    if (scriptedActions.startedTimerPlotIndex != scriptedActions.currentPlotIndex)
    {
        isNewStep = 1;
        scriptedActions.plotIndexStartTime = ctx->time;
        scriptedActions.startedTimerPlotIndex = scriptedActions.currentPlotIndex;
    }
    
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        ScriptedAction action = scriptedActions.actions[i];
        if (action.actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            continue;
        }


        if (action.startPlotIndex > scriptedActions.currentPlotIndex
            || action.endPlotIndex < scriptedActions.currentPlotIndex)
        {
            continue;
        }

        if (isNewStep && action.actionStartTime == 0.0f)
        {
            scriptedActions.actions[i].actionStartTime = ctx->time;
            action.actionStartTime = ctx->time;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SPEECH_BUBBLE)
        {
            int16_t characterX = player.x;
            int16_t characterY = player.y;
            float chrX, chrY;
            if (Enemies_getPosition(action.speechBubble.speaker, &chrX, &chrY))
            {
                characterX = (int16_t)chrX;
                characterY = (int16_t)chrY;
            }

            DrawSpeechBubble(screenData, action.speechBubble.speechBubbleX, action.speechBubble.speechBubbleY, action.speechBubble.speechBubbleWidth, action.speechBubble.speechBubbleHeight, 
                characterX + action.speechBubble.arrowXOffset, characterY + action.speechBubble.arrowYOffset, action.speechBubble.text);
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_PLAYER_CONTROLS_ENABLED)
        {
            Player_setInputEnabled(action.playerControlsData.enabled);
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_PROCEED_PLOT_CONDITION)
        {
            if (Condition_update(&action.proceedPlotCondition.condition, ctx, screenData, ctx->time - scriptedActions.plotIndexStartTime))
            {
                TE_Logf("SCRIPTED_ACTION", "Condition met, proceeding plot to %d", action.proceedPlotCondition.setPlotIndex);
                nextPlotIndex = action.proceedPlotCondition.setPlotIndex;
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_NPC_TARGET)
        {
            Enemies_setTarget(
                action.npcTarget.id, 
                action.npcTarget.x, 
                action.npcTarget.y);
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_PLAYER_TARGET || action.actionType == SCRIPTED_ACTION_TYPE_SET_PLAYER_POSITION)
        {
            if (action.playerTarget.setX)
            {
                playerCharacter.targetX = action.playerTarget.targetX;
                if (action.actionType == SCRIPTED_ACTION_TYPE_SET_PLAYER_POSITION)
                {
                    playerCharacter.x = action.playerTarget.targetX;
                }
            }
            if (action.playerTarget.setY)
            {
                playerCharacter.targetY = action.playerTarget.targetY;
                if (action.actionType == SCRIPTED_ACTION_TYPE_SET_PLAYER_POSITION)
                {
                    playerCharacter.y = action.playerTarget.targetY;
                }
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_FLAGS)
        {
            nextFlags = (nextFlags & ~action.setFlags.mask) | action.setFlags.setFlags;
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_NPC_HEALTH)
        {
            Enemies_setHealth(action.npcHealth.id, action.npcHealth.health);
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_ENEMY_CALLBACK)
        {
            Enemy* enemy = Enemies_getEnemy(action.setEnemyCallback.id, 0);
            if (enemy) {
                LOG("Setting enemy[%d] (%p) callback", action.setEnemyCallback.id, enemy);
                enemy->userCallbackData = action.setEnemyCallback.callback;
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SCENE_FADE_OUT)
        {
            float fadeTime = ctx->time - scriptedActions.plotIndexStartTime - action.sceneFadeOut.beginDelay;
            float fadePercent = fadeTime / action.sceneFadeOut.duration;
            // TE_Logf("SCRIPTED_ACTION", "Fading out %.2f (%.2f:%.2f)", fadePercent, fadeTime, action.sceneFadeOut.duration);
            if (fadePercent < 0.0f)
            {
                fadePercent = 0.0f;
            }

            if (fadeTime > action.sceneFadeOut.duration + action.sceneFadeOut.finishDelay + action.sceneFadeOut.beginDelay)
            {
                TE_Logf("SCRIPTED_ACTION", "Fading out done, going to %d (t=%.2f [%.2f; %.2f; %.2f])", 
                    action.sceneFadeOut.nextPlotIndex, fadeTime, action.sceneFadeOut.duration, action.sceneFadeOut.finishDelay, action.sceneFadeOut.beginDelay);
                nextPlotIndex = action.sceneFadeOut.nextPlotIndex;
            }
            if (fadePercent > 1.0f)
            {
                fadePercent = 1.0f;
            }
            if (action.sceneFadeOut.type & FADEIN_FLAG)
            {
                fadePercent = 1.0f - fadePercent;
            }
            switch (action.sceneFadeOut.type & ~FADEIN_FLAG)
            {
                case FADEOUT_LEFT_TO_RIGHT:
                    TE_Img_fillRect(screenData, 0, 0, (int16_t)(fadePercent * 128.0f), 128, DB32Colors[0], (TE_ImgOpState){
                        .zCompareMode = Z_COMPARE_ALWAYS,
                        .zValue = 255,
                    });
                    break;
                case FADEOUT_RIGHT_TO_LEFT:
                    TE_Img_fillRect(screenData, 128 - (int16_t)(fadePercent * 128.0f), 0, (int16_t)(fadePercent * 128.0f), 128, DB32Colors[0], (TE_ImgOpState){
                        .zCompareMode = Z_COMPARE_ALWAYS,
                        .zValue = 255,
                    });
                    break;
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_TITLE_SCREEN)
        {
            if (action.titleScreen.fillBlackBackground)
            {
                TE_Img_clear(screenData, DB32Colors[0], 0);
            }
            TE_Font largeFont = GameAssets_getFont(FONT_LARGE);
            int16_t centerY = 60;
            TE_Font_drawTextBox(screenData, &largeFont, 4, centerY - 40, 120, 35, -1, -4, action.titleScreen.titleText, 0.5f, 1.0f, 0xffffffff, (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 255,
            });
            TE_Font mediumFont = GameAssets_getFont(FONT_MEDIUM);
            TE_Font_drawTextBox(screenData, &mediumFont, 4, centerY + 5, 120, 80, -1, -4, action.titleScreen.subText, 0.5f, 0.0f, 0xffffffff, (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 255,
            });

            TE_Img_fillRect(screenData, 4, centerY, 120, 1, 0xffffffff, (TE_ImgOpState){
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 255,
            });

            DrawNextButtonAction(ctx, screenData);
            if (ctx->inputRight && !ctx->prevInputRight)
            {
                nextPlotIndex = action.titleScreen.nextPlotIndex;
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_LOAD_SCENE)
        {
            TE_Logf("SCRIPTED_ACTION", "Loading scene %d", action.loadScene.sceneId);
            _loadNextScene = action.loadScene.sceneId;
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_CLEAR_SCREEN)
        {
            TE_Img_clear(screenData, action.clearScreen.color, action.clearScreen.z);
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_NPC_SPAWN)
        {
            if (!Enemies_isAlive(action.npcSpawn.id) && isNewStep)
            {
                Enemies_spawn(action.npcSpawn.id, action.npcSpawn.characterType, action.npcSpawn.x, action.npcSpawn.y);
                Enemies_setTarget(action.npcSpawn.id, action.npcSpawn.targetX, action.npcSpawn.targetY);
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_CUSTOM_CALLBACK)
        {
            action.customCallback.callback(ctx, screenData, &action);
            scriptedActions.actions[i] = action;
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_SET_ITEM)
        {
            if (action.setItem.charId == 0)
            {
                Player_setWeapon(action.setItem.rightItemIndex);
            }
            else
            {
                Enemies_setItem(action.setItem.charId, action.setItem.leftItemIndex, action.setItem.rightItemIndex);
            }
            continue;
        }

        if (action.actionType == SCRIPTED_ACTION_TYPE_ANIMATION_PLAYBACK)
        {
            float t = ctx->time - action.actionStartTime;
            if (action.animationPlayback.delay <= t)
            {
                t = (t - action.animationPlayback.delay) * action.animationPlayback.speed;
                uint32_t msTick = (uint32_t)(t * 1000.0f);
                GameAssets_drawAnimation(action.animationPlayback.animationId, screenData, msTick,
                    action.animationPlayback.x, action.animationPlayback.y, action.animationPlayback.maxLoopCount,
                    (BlitEx) {
                        .tint = action.animationPlayback.tintColor != 0xffffffff,
                        .tintColor = action.animationPlayback.tintColor,
                        .blendMode = TE_BLEND_ALPHAMASK,
                        .state.zValue = action.animationPlayback.z,
                    });
            }
            continue;
        }
    }

    scriptedActions.currentPlotIndex = nextPlotIndex;
    scriptedActions.flags = nextFlags;
    TE_randSetSeed(oldSeed);
}

static void (*_sceneUpdateFn)(RuntimeContext *ctx, TE_Img *screenData);

static void DrawTextBlock(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, const char *text)
{
    TE_Font font = GameAssets_getFont(0);
    TE_Img_fillRect(screenData, x+1, y+1, width-2, height-2, DB32Colors[21], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Img_lineRect(screenData, x, y, width, height, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Font_drawTextBox(screenData, &font, x + 4, y + 4, width - 8, height - 8, -1, -4, text, 0.5f, 0.5f, 0xffffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}

void DrawSpeechBubble(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, int16_t arrowX, int16_t arrowY, const char *text)
{
    TE_Font font = GameAssets_getFont(0);
    // TE_Img_drawPatch9(screenData, &font.atlas, x, y, width, height, 0, 0, 0, 0, (BlitEx){
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //         .zValue = 0,
    //     }
    // });
    int baseW = 8;
    int cx = arrowX > x + baseW * 2 ? (arrowX < x + width - baseW * 2 ? arrowX : x + width - baseW * 2) : x + baseW * 2;
    int cy = y + height / 2;
    TE_Img_fillTriangle(screenData, arrowX, arrowY, cx - baseW, cy, cx + baseW, cy, DB32Colors[21], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Img_lineTriangle(screenData, arrowX, arrowY, cx - baseW, cy, cx + baseW, cy, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    

    TE_Img_lineRect(screenData, x, y, width, height, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS,
        .zValue = 255,
    });
    TE_Img_fillRect(screenData, x+1, y+1, width-2, height-2, DB32Colors[21], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    // printf("Drawing text: %s %p\n", text, font.atlas->data);
    TE_Font_drawTextBox(screenData, &font, x + 4, y + 4, width - 8, height - 8, -1, -4, text, 0.5f, 0.5f, 0xffffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}

static void Cart_draw(TE_Img *screenData, int16_t x, int16_t y, uint8_t loaded, RuntimeContext *ctx)
{
    TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_CART_SIDE), x, y, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = y + 8,
        }
    });
    switch(loaded)
    {
        case 1:
            TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_CART_GOLD), x, y-3, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = y + 8,
                }
            });
            TE_randSetSeed(ctx->frameCount / 4);
            int sparks = TE_randRange(1, 3);
            for (int i=0;i<sparks;i++)
            {
                int sparkX = TE_randRange(x-7, x+1);
                int sparkY = TE_randRange(y-5, y+1);
                TE_Img_setPixel(screenData, sparkX, sparkY, DB32Colors[21], (TE_ImgOpState){
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = y+8,
                });
            }
        break;

    }
    TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_CART_WHEEL_SIDE), x-1, y+5, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = y + 8,
        }
    });
    
    TE_Img_fillRect(screenData, x-7, y+4, 16, 4, DB32Colors[14], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 0,
    });
}

static void Scene_1_update(RuntimeContext *ctx, TE_Img *screenData)
{
    // Update scene 1
    int16_t cartX = player.x - 20;
    
    // draw path
    const uint16_t pathQHeight = 16;
    for (uint16_t x=0;x<128;x+=2)
    {
        uint16_t srcX = x % 16;
        int16_t y = 50 + (int16_t)(sin((x) * 0.05f) * 5.0f + x * 0.1f);

        TE_Img_blitEx(screenData, &atlasImg, x, y, 71 + srcX, 16, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x, y+pathQHeight, 71 + srcX, 16 + 24, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
    }
    // TE_Img_drawPatch9(screenData, &atlasImg, -8, 58, 140, 28, 64,16, 12, 12, (BlitEx){
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //         .zValue = 0,
    //     }
    // });

    int16_t cartY = 60 + (int16_t)(sin((cartX) * 0.05f) * 5.0f + cartX * 0.1f);
    int16_t cartYAnchor = 60 + (int16_t)(sin((cartX+6) * 0.05f) * 5.0f + (cartX + 6) * 0.1f);

    player.y = 54 + (int16_t)(sin((player.x) * 0.05f) * 5.0f + player.x * 0.1f);
    player.aimY = 1;
    player.dirY = 1;
    player.dy = 1;
    playerCharacter.y = player.y;
    playerCharacter.dirY = 1;

    Cart_draw(screenData, cartX, cartY,1, ctx);

    TE_Img_line(screenData, cartX+8, cartYAnchor, cartX+20, player.y + 6, DB32Colors[3], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });
    TE_Img_line(screenData, cartX+8, cartYAnchor + 1, cartX+20, player.y + 7, DB32Colors[2], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });

    const char *text = NULL;

    if (player.x > 108)
    {
        text = NULL;
    }
    else if (player.x > 80)
    {
        text = "I can now get the pool I always wanted!";
    }
    else if (player.x > 60)
    {
        text = "Finally I can retire.";
    }
    else if (player.x > 40)
    {
        text = "So. Much. Loot!";
    }
    else if (player.x > 20)
    {
        text = "The crusades were awesome.";
    }

    if (text)
    {
        DrawSpeechBubble(screenData, 10,20, 108, 30, cartX + 20, player.y - 5, text);
    }

    if (!ctx->inputRight)
    {
        DrawTextBlock(screenData, 70, 105, 48, 16, "Press >");
    }

    if (player.x < -8)
    {
        player.x = -8;
        playerCharacter.x = player.x;
    }
    if (cartX > 130)
    {
        Scene_init(2);
    }
}

static void Scene_1_init()
{
    // Enemies_spawn(1, 28, 42);
    // Enemies_spawn(1, 44, 28);

    player.x = -8;
    playerCharacter.x = player.x;

    Environment_addTreeGroup(24, 30, 26, 5, 25);
    Environment_addTreeGroup(114, 30, 122, 5, 25);
    Environment_addTreeGroup(114, 125, 1252, 5, 25);
    Environment_addTreeGroup(24, 124, 99, 5, 20);
    // Environment_addTreeGroup(64, 84, 199, 3, 20);
}

#define SCENE_2_FLAG_PULLING_CART 1
#define SCENE_2_FLAG_DINGLEWORT_OUTSIDE 2
#define SCENE_2_FLAG_DOOR_OPEN 4

static void Scene_2_init()
{
    Environment_addBushGroup(112, 90, 1232, 5, 10);

    Environment_addTreeGroup(24, 120, 122, 5, 25);
    Environment_addTreeGroup(104, 10, 1522, 5, 35);
    Environment_addTreeGroup(54, 20, 1622, 5, 20);
    Environment_addTreeGroup(10, 15, 522, 5, 25);
    Environment_addTree(118, 125, 5122);
    Environment_addFlowerGroup(60,110, 232, 15, 20);

    player.x = -15;
    playerCharacter.x = player.x;
    Enemies_init();
    Enemies_spawn(1, 1, 102,66);
    Enemies_spawn(2, 1, 77,42);
    Enemies_spawn(3, 2, 96,43);

    Enemies_setItem(1, ITEM_PIKE, 0);
    Enemies_setItem(2, ITEM_PIKE, 0);

    ScriptedAction_addProceedPlotCondition(0, 0, 1, (Condition){
        .type = CONDITION_TYPE_PLAYER_IN_RECT,
        .npcsInRect.x = 40,
        .npcsInRect.y = 0,
        .npcsInRect.width = 20,
        .npcsInRect.height = 128,
    });

    scriptedActions.flags = SCENE_2_FLAG_PULLING_CART;

    uint8_t step = 0;

    ScriptedAction_addSceneFadeOut(step, step, FADEIN_LEFT_TO_RIGHT, step + 1, .85f, 0.75f, 0.0f);
    step++;

    ScriptedAction_addPlayerControlsEnabled(step, step, 0);
    ScriptedAction_addSpeechBubble(step, step, "Ah, my castle!", 0, 8, 96, 112, 28, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addPlayerControlsEnabled(step, step, 0);
    ScriptedAction_addSetPlayerTarget(step, step, 8, 60, 1, 0);
    ScriptedAction_addSetPlayerTarget(step, step, 15, 60, 1, 0);
    ScriptedAction_addSpeechBubble(step, step, "What a weird looking banner...", 0, 8, 96, 112, 28, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addPlayerControlsEnabled(step, step, 0);
    ScriptedAction_addSetPlayerTarget(step, step, 40, 60, 1, 0);
    ScriptedAction_addSetNPCTarget(step, step, 3, 80, 50);
    ScriptedAction_addSetFlags(step, step, SCENE_2_FLAG_DOOR_OPEN, SCENE_2_FLAG_DOOR_OPEN);
    ScriptedAction_addSpeechBubble(step, step, "Why does it say 'For Sale'?!", 0, 8, 96, 112, 28, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Welcome, Sire!", 3, 8, 4, 112, 28, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;
    
    ScriptedAction_addSpeechBubble(step, step, "Who are you??", 0, 8, 96, 112, 28, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "I am Sir Clodrick Dinglewort.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;
    
    ScriptedAction_addSpeechBubble(step, step, "What are you doing in my Castle, Sir Dinglewhat?!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Your castle? Oh, you brought the cash - ", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "- so you could say that.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "What? No, this is my castle, ", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "I am SIR ROBIN", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;
    
    ScriptedAction_addSpeechBubble(step, step, "...", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Oh, in that case, we'll accept your gold ...", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "... as down payment to your tax pay debts.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addSetNPCTarget(step, step, 1, 25, 70);
    ScriptedAction_addSetNPCTarget(step, step, 2, 25, 55);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_NPCS_IN_RECT, 
        .npcsInRect.npcIds = {1,2,0,0}, .npcsInRect.x = 20, .npcsInRect.y = 50, .npcsInRect.width = 20, .npcsInRect.height = 30 });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "You have no right, this is my loot!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "As the tax collector of Nottingham,", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addSetNPCTarget(step, step, 1, -25, 70 + 15);
    ScriptedAction_addSetNPCTarget(step, step, 2, -25, 55 + 16);
    ScriptedAction_addSetFlags(step, step, 0, SCENE_2_FLAG_PULLING_CART);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_NPCS_IN_RECT, 
        .npcsInRect.npcIds = {1,2,0,0}, .npcsInRect.x = -30, .npcsInRect.y = 0, .npcsInRect.width = 10, .npcsInRect.height = 128 });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "I am obliged to do that.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "It's nothing personal.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "OK fine, then leave now my home and castle.", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;
    
    ScriptedAction_addSpeechBubble(step, step, "You are under the wrong impression.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "You owe the crown much more gold.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "WHAT!? This isn't right!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "What is right and what is wrong ...", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "... is up to the Sheriff to decide.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "I look forward to meet you again.", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addSetNPCTarget(step, step, 3, 96, 43);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "With the rest of your due taxes!", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addSetFlags(step, step, 0, SCENE_2_FLAG_DOOR_OPEN);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;
    
    ScriptedAction_addSpeechBubble(step, step, "What just happened?!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "And where's my LOOT!?!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addSetPlayerTarget(step, step, 20, 60, 1, 0);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "Dingleduck is going to pay for that.", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "First I'll get my loot back!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addSetPlayerTarget(step, step, -50, 60, 1, 0);
    ScriptedAction_addSetNPCHealth(step, step, 1, 0);
    ScriptedAction_addSetNPCHealth(step, step, 2, 0);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PLAYER_IN_RECT, 
        .npcsInRect.x = -40, .npcsInRect.y = 0, .npcsInRect.width = 20, .npcsInRect.height = 128 });
    step++;
    
    ScriptedAction_addSceneFadeOut(step, step, FADEOUT_RIGHT_TO_LEFT, step + 1, 1.5f, 0.0f, 1.0f);
    step++;

    ScriptedAction_addTitleScreen(step, step + 1, "Part 1", "Where is my loot?", 1, step + 1);
    step++;

    ScriptedAction_addLoadScene(step, step, SCENE_3_CHASING_THE_LOOT);
}

static void DrawTower(TE_Img *screenData, int16_t x, int16_t y, uint8_t z)
{
    TE_Img_blitEx(screenData, &atlasImg, x, y - 14, 114, 97, 28, 24, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = z,
        }
    });
    TE_Img_blitEx(screenData, &atlasImg, x+2, y + 4, 116, 128, 24, 37, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = z,
        }
    });
}
static void Scene_2_update(RuntimeContext *ctx, TE_Img *screenData)
{
    // Update scene 2
    // DrawTextBlock(screenData, 10, 10, 108, 30, "Scene 2");

    int16_t castleX = 108;

    DrawTower(screenData, castleX+10, 64, 120);

    int16_t cartX = player.x - 20;
    int16_t cartY = 78 - (int16_t)(sin((cartX * 1.2f+12.0f) * 0.05f) * 5.0f + cartX * 0.2f);
    int16_t cartYAnchor = 80 - (int16_t)(sin(((cartX+6.0f) * 1.2f+12.0f) * 0.05f) * 5.0f + (cartX+6.0f) * 0.2f);

    uint8_t isPullingCart = (scriptedActions.flags & SCENE_2_FLAG_PULLING_CART) != 0;

    player.y = 74 - (int16_t)(sin((player.x * 1.2f+12.0f) * 0.05f) * 5.0f + player.x * 0.2f);
    player.aimY = 1;
    player.dirY = 1;
    player.dy = 1;
    playerCharacter.y = player.y;
    playerCharacter.dirY = 1;
    playerCharacter.targetY = player.y;

    if (!isPullingCart)
    {
        float x1, y1, x2, y2;
        Enemies_getPosition(1, &x1, &y1);
        Enemies_getPosition(2, &x2, &y2);
        float cx = (x1 + x2) * 0.5f;
        float cy = (y1 + y2) * 0.5f;
        cartX = (int16_t)cx - 4;
        cartY = (int16_t)cy + 6;
    }

    Cart_draw(screenData, cartX, cartY,1, ctx);

    if (isPullingCart)
    {
        TE_Img_line(screenData, cartX+8, cartYAnchor, cartX+20, player.y + 4, DB32Colors[3], (TE_ImgOpState) {
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = 83,
        });
        TE_Img_line(screenData, cartX+8, cartYAnchor + 1, cartX+20, player.y + 5, DB32Colors[2], (TE_ImgOpState) {
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = 83,
        });
    }


    const uint8_t castleZ = 70;

    // building
    TE_Img_blitEx(screenData, &atlasImg, castleX-8, 10, 0, 144, 39, 45, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = castleZ - 12,
            }
        });
    TE_Img_blitEx(screenData, &atlasImg, castleX-5, 2, 39, 144, 16, 31, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = castleZ - 12,
            }
        });
    

    // wall fronts left
    for (int i=0;i<4;i++)
    {
        TE_Img_blitEx(screenData, &atlasImg, 
            castleX+16*i-48, 16*i + (i==0?16:0), 
            i == 0 ? 144 : 160, 113 + (i == 0 ? 16 : 0), 16, 
            47 - (i == 0 ? 16 : 0), (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = castleZ + i * 12 - 25,
            }
        });
    }

    // door
    TE_Img_blitEx(screenData, &atlasImg, castleX-25, 40, 144, 160, 17, 36, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = castleZ - 6,
            }
        });
    
    if (scriptedActions.flags & SCENE_2_FLAG_DOOR_OPEN)
    {
        // Enemies_setTarget(3, 80,50);
        // open door state: open part
        TE_Img_blitEx(screenData, &atlasImg, castleX-23, 42, 169, 160, 8, 21, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_ALWAYS,
                    .zValue = castleZ - 11,
                }
            });
        // swung open door 
        TE_Img_blitEx(screenData, &atlasImg, castleX-23-8, 41, 161, 160, 8, 18, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = castleZ - 6,
                }
            });
    }

    // coat of arms
    TE_Img_blitEx(screenData, &atlasImg, castleX-19, 35, 144, 112, 5, 12, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = castleZ - 1,
            }
        });

    // big flag
    for (int y = 0; y < 24; y++)
    {
        int16_t xoffset = (int) (sinf(-ctx->time * 3.5f + y * .25f) * ((y-3) * 0.165f)-.5f);
        TE_Img_blitEx(screenData, &atlasImg, castleX-20 + xoffset, 20+y, 192, 112+y, 9, 1, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = castleZ,
            }
        });
    }

    DrawTower(screenData, castleX-54, 1,castleZ - 30);

    // backside wall
    for (int i=0;i<4;i++)
    {
        TE_Img_blitEx(screenData, &atlasImg, castleX+16*i-28, -8*i-18, 176, 113, 16, 47, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = 10,
            }
        });
    }

    // draw path
    const uint16_t pathQHeight = 14;
    for (uint16_t x=0;x<100;x+=2)
    {
        uint16_t srcX = x % 16;
        int16_t y = 65 - (int16_t)(sin((x * 1.2f+12.0f) * 0.05f) * 5.0f + x * 0.2f);

        TE_Img_blitEx(screenData, &atlasImg, x, y, 71 + srcX, 16, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x, y+pathQHeight, 71 + srcX, 16 + 24, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
    }


    const char *text = NULL;
    
    if (player.x < 5)
    {
        text = "Ah, my castle!";
    }
    else if(player.x < 10)
    {
        text = "What a weird looking banner...";
    }
    else if(player.x < 20)
    {
        text = "Why does it say 'For Sale'?!";
    }

    if (text)
    {
        // DrawSpeechBubble(screenData, 5,98, 118, 28, player.x, player.y + 18, text);
    }

}

#include "game_scene_0_testing.h"
#include "game_scene_3.h"

static const Scene scenes[] = {
    { .id = SCENE_0_TESTING, .initFn = Scene_0_init, .updateFn = Scene_0_update },
    { .id = SCENE_1_PULLING_THE_CART, .initFn = Scene_1_init, .updateFn = Scene_1_update },
    { .id = SCENE_2_ARRIVING_AT_HOME, .initFn = Scene_2_init, .updateFn = Scene_2_update },
    { .id = SCENE_3_CHASING_THE_LOOT, .initFn = Scene_3_init, .updateFn = Scene_3_update },
    {0}
};

static void NoSceneUpdate(RuntimeContext *ctx, TE_Img *screenData)
{
    // Do nothing
    DrawTextBlock(screenData, 10, 10, 108, 30, "No scene loaded");
}

static uint8_t _currentSceneId = 0xff;

// a simple allocator for scene data to avoid dynamic memory allocation
// does not allow freeing memory, is reset on scene change
static uint8_t _sceneAllocatorData[0x30000];
static uint32_t _sceneAllocatorOffset = 0;

// malloc from the scene memory. Memory is zero initialized and not freeable. Will reset on scene change.
void* Scene_malloc(uint32_t size)
{
    if (size == 0) return 0;
    if (_sceneAllocatorOffset + size > sizeof(_sceneAllocatorData))
    {
        LOG("Can not allocate %d bytes, out of memory (%d)", size, sizeof(_sceneAllocatorData));
        TE_Panic("Out of memory");
        return NULL;
    }
    void *ptr = &_sceneAllocatorData[_sceneAllocatorOffset];
    _sceneAllocatorOffset += size;
    _sceneAllocatorOffset = ALIGN_VALUE4(_sceneAllocatorOffset);
    return ptr;
}

// duplicate a string into the scene memory
char* Scene_strDup(const char *str, int strlength)
{
    if (!str) return NULL;
    char *ptr = Scene_malloc(strlength + 1);
    memcpy(ptr, str, strlength);
    return ptr;
}

uint32_t Scene_getAllocatedSize()
{
    return _sceneAllocatorOffset;
}

void Scene_init(uint8_t sceneId)
{
    _sceneAllocatorOffset = 0;
    memset(_sceneAllocatorData, 0, sizeof(_sceneAllocatorData));
    _currentSceneId = sceneId;
    TE_Logf("SCENE", "Init scene %d", sceneId);
    RenderObject_init(0x8000);
    ParticleSystem_init();
    Enemies_init();
    Player_setInputEnabled(1);
    ScriptedAction_init();
    Player_setWeapon(0);
    Environment_init();
    player.x = 64;
    player.y = 64;
    playerCharacter.x = player.x;
    playerCharacter.y = player.y;

    for (int i=0;scenes[i].initFn;i++)
    {
        if (scenes[i].id == sceneId)
        {
            scenes[i].initFn();
            _sceneUpdateFn = scenes[i].updateFn;
            return;
        }
    }

    _sceneUpdateFn = NoSceneUpdate;
}

void Scene_update(RuntimeContext *ctx, TE_Img *screen)
{
    if (_loadNextScene >= 0)
    {
        Scene_init(_loadNextScene);
        _loadNextScene = -1;
    }
    _sceneUpdateFn(ctx, screen);
}

uint8_t Scene_getCurrentSceneId()
{
    return _currentSceneId;
}

uint8_t Scene_getMaxStep()
{
    uint8_t maxStep = 0;
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType != SCRIPTED_ACTION_TYPE_NONE && 
            scriptedActions.actions[i].endPlotIndex > maxStep)
        {
            maxStep = scriptedActions.actions[i].endPlotIndex;
        }
    }
    return maxStep;
}

void Scene_setStep(uint8_t step)
{
    scriptedActions.currentPlotIndex = step;
}

uint8_t Scene_getStep()
{
    return scriptedActions.currentPlotIndex;
}