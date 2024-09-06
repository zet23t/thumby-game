#include "game_scene_3.h"
#include "game_player.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_environment.h"
#include "game_assets.h"
#include "game.h"

#include "TE_rand.h"
#include <math.h>

void Scene_3_enemyTookDamage(struct Enemy *enemy, float damage, float vx, float vy);

void Scene_3_init()
{
    Environment_addTreeGroup(20, 20, 18522, 4, 20);
    Environment_addTreeGroup(100, 30, 1852, 6, 25);
    Environment_addFlowerGroup(76,40, 232, 15, 20);
    Environment_addBushGroup(75,35, 1232, 5, 10);

    player.x = 160;
    playerCharacter.x = player.x;
    player.y = 110;
    playerCharacter.y = player.y;

    uint8_t step = 0;

    ScriptedAction_addSetPlayerTarget(step, step, 90, 73, 1, 1);
    ScriptedAction_addPlayerControlsEnabled(step, step, 0);
    ScriptedAction_addSceneFadeOut(step, step, FADEIN_RIGHT_TO_LEFT, step + 1, 0.85f, 0.4f, 1.0f);
    step++;

    ScriptedAction_addSpeechBubble(step, step, "The shortcut over the stream. That way I can catch up!", 0, 8, 4, 112, 38, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addNPCSpawn(step, step, 1, 3, -5, 60, 60, 70);
    ScriptedAction_addNPCSpawn(step, step, 2, 4, -20, 60, 50, 66);
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

    
    ScriptedAction_addNPCSpawn(step, step, 3, 3, 100, 0, 100, 40);
    ScriptedAction_addNPCSpawn(step, step, 4, 4, 130, 120, 100, 96);
    ScriptedAction_addPlayerControlsEnabled(step, step, 1);
    ScriptedAction_addSetItem(step, step, 0, 0, ITEM_STAFF);
    ScriptedAction_addSetItem(step, step, 1, 0, ITEM_STAFF);
    ScriptedAction_addSetItem(step, step, 2, -ITEM_STAFF, 0);
    ScriptedAction_addSetItem(step, step, 3, 0, -ITEM_STAFF);
    ScriptedAction_addSetItem(step, step, 4, 0, ITEM_STAFF);

    ScriptedAction_addSpeechBubble(step, step, "Great, the more the merrier!", 0, 4, 4, 70, 48, -4, -8);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_WAIT, .wait.duration = 2.5f });
    step++;

    static int enemyAliveCount = 4;
    TookDamageCallbackData tookDamageCallbackData = {
        .callback = &Scene_3_enemyTookDamage,
        .dataInt = step + 2,
        .dataPointer = &enemyAliveCount,
    };
    ScriptedAction_addSetEnemyCallback(step, step, 1, tookDamageCallbackData);
    ScriptedAction_addSetEnemyCallback(step, step, 2, tookDamageCallbackData);
    ScriptedAction_addSetEnemyCallback(step, step, 3, tookDamageCallbackData);
    ScriptedAction_addSetEnemyCallback(step, step, 4, tookDamageCallbackData);
    ScriptedAction_addJumpStep(step, step, step + 1);
    step++;
    // fight
    step++;
    LOG("final Step: %d", step);
    ScriptedAction_addSpeechBubble(step, step, "Aaaand I am done here.", 0, 4, 4, 70, 48, -4, -8);

}

void Scene_3_drawKOEnemy(RuntimeContext *ctx, TE_Img *screenData, ScriptedAction *action)
{
    Enemy *enemy = action->customCallback.enemyPointer;
    Character_drawKO(screenData, &enemy->character);
}

void Scene_3_enemyTookDamage(struct Enemy *enemy, float damage, float vx, float vy)
{
    LOG("Callback: Enemy took damage %f", damage);
    if (enemy->health <= 0.0f)
    {
        int *enemyAliveCount = (int*)enemy->damageCallbackData.dataPointer;
        (*enemyAliveCount)--;
        LOG("Enemey eliminated, remaining: %d", *enemyAliveCount);
        if (*enemyAliveCount == 0)
        {
            LOG("All enemies eliminated, proceeding to %d", enemy->damageCallbackData.dataInt);
            Scene_setStep(enemy->damageCallbackData.dataInt);
        }
        ScriptedAction* action = ScriptedAction_addCustomCallback(0, 0xff, Scene_3_drawKOEnemy);
        action->customCallback.dataPointer = enemy;
    }
}

void Scene_3_update(RuntimeContext *ctx, TE_Img *screenData)
{
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
        if (p % 13 == 5)
        {
            TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_POLE_TOP), x + 2, y + 1, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = 2,
                }
            });
            TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_POLE_TOP), x, y + 14, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = 4,
                }
            });
        }
    }

    
}