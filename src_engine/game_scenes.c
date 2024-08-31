#include "game_scenes.h"
#include "game_enemies.h"
#include "game_player.h"
#include "game_assets.h"
#include "game_environment.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>

static void DrawSpeechBubble(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, int16_t arrowX, int16_t arrowY, const char *text);

typedef struct Condition
{
    uint8_t type;
    union {
        struct RectCondition {
            int16_t x;
            int16_t y;
            int16_t width;
            int16_t height;
        };
    };
} Condition;

#define CONDITION_TYPE_PLAYER_IN_RECT 1
#define CONDITION_TYPE_PRESS_NEXT 2

uint8_t Condition_update(Condition *condition, RuntimeContext *ctx, TE_Img *screenData)
{
    if (condition->type == CONDITION_TYPE_PLAYER_IN_RECT)
    {
        return (player.x >= condition->x && player.x < condition->x + condition->width
            && player.y >= condition->y && player.y < condition->y + condition->height);
    }

    if (condition->type == CONDITION_TYPE_PRESS_NEXT)
    {
        return ctx->inputRight && !ctx->prevInputRight;
    }

    return 0;
}


typedef struct ScriptedAction
{
    int16_t startPlotIndex;
    int16_t endPlotIndex;
    uint8_t actionType;
    union {
        struct SpeechBubbleData {
            const char *text;
            uint8_t speaker;
            uint8_t speechBubbleX;
            uint8_t speechBubbleY;
            uint8_t speechBubbleWidth;
            uint8_t speechBubbleHeight;
            int8_t arrowXOffset;
            int8_t arrowYOffset;
        };
        struct PlayerControlsData {
            uint8_t enabled;
        };
        struct ProceedPlotConditionData {
            uint8_t setPlotIndex;
            Condition condition;
        };
        struct SetNPCTargetData {
            uint8_t id;
            int16_t x;
            int16_t y;
        };
    };
} ScriptedAction;

#define MAX_SCRIPTED_ACTIONS 64

typedef struct ScriptedActions
{
    ScriptedAction actions[MAX_SCRIPTED_ACTIONS];
    uint8_t currentPlotIndex;
} ScriptedActions;

struct ScriptedActions scriptedActions;

#define SCRIPTED_ACTION_TYPE_NONE 0
#define SCRIPTED_ACTION_TYPE_SPEECH_BUBBLE 1
#define SCRIPTED_ACTION_TYPE_SET_PLAYER_CONTROLS_ENABLED 2
#define SCRIPTED_ACTION_TYPE_PROCEED_PLOT_CONDITION 3
#define SCRIPTED_ACTION_TYPE_SET_NPC_TARGET 4

void ScriptedAction_init()
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_NONE;
    }
    scriptedActions.currentPlotIndex = 0;
}

void ScriptedAction_addSpeechBubble(uint8_t stepStart, uint8_t stepStop, const char *text, uint8_t speaker, uint8_t speechBubbleX, uint8_t speechBubbleY, uint8_t speechBubbleWidth, uint8_t speechBubbleHeight, int8_t arrowXOffset, int8_t arrowYOffset)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SPEECH_BUBBLE;
            scriptedActions.actions[i].text = text;
            scriptedActions.actions[i].speaker = speaker;
            scriptedActions.actions[i].speechBubbleX = speechBubbleX;
            scriptedActions.actions[i].speechBubbleY = speechBubbleY;
            scriptedActions.actions[i].speechBubbleWidth = speechBubbleWidth;
            scriptedActions.actions[i].speechBubbleHeight = speechBubbleHeight;
            scriptedActions.actions[i].arrowXOffset = arrowXOffset;
            scriptedActions.actions[i].arrowYOffset = arrowYOffset;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_addPlayerControlsEnabled(uint8_t stepStart, uint8_t stepStop, uint8_t enabled)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_PLAYER_CONTROLS_ENABLED;
            scriptedActions.actions[i].enabled = enabled;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_setNPCTarget(uint8_t stepStart, uint8_t stepStop, uint8_t id, int16_t x, int16_t y)
{
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            scriptedActions.actions[i].actionType = SCRIPTED_ACTION_TYPE_SET_NPC_TARGET;
            scriptedActions.actions[i].id = id;
            scriptedActions.actions[i].x = x;
            scriptedActions.actions[i].y = y;
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
            scriptedActions.actions[i].setPlotIndex = setPlotIndex;
            scriptedActions.actions[i].condition = condition;
            scriptedActions.actions[i].startPlotIndex = stepStart;
            scriptedActions.actions[i].endPlotIndex = stepStop;
            return;
        }
    }
}

void ScriptedAction_update(RuntimeContext *ctx, TE_Img *screenData)
{
    uint8_t nextPlotIndex = scriptedActions.currentPlotIndex;
    for (int i=0;i<MAX_SCRIPTED_ACTIONS;i++)
    {
        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_NONE)
        {
            continue;
        }

        if (scriptedActions.actions[i].startPlotIndex > scriptedActions.currentPlotIndex
            || scriptedActions.actions[i].endPlotIndex < scriptedActions.currentPlotIndex)
        {
            continue;
        }

        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_SPEECH_BUBBLE)
        {
            ScriptedAction action = scriptedActions.actions[i];
            int16_t characterX = player.x;
            int16_t characterY = player.y;
            float chrX, chrY;
            if (Enemies_getPosition(action.speaker, &chrX, &chrY))
            {
                characterX = (int16_t)chrX;
                characterY = (int16_t)chrY;
            }

            DrawSpeechBubble(screenData, action.speechBubbleX, action.speechBubbleY, action.speechBubbleWidth, action.speechBubbleHeight, 
                characterX + action.arrowXOffset, characterY + action.arrowYOffset, action.text);
            continue;
        }

        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_SET_PLAYER_CONTROLS_ENABLED)
        {
            Player_setInputEnabled(scriptedActions.actions[i].enabled);
            continue;
        }

        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_PROCEED_PLOT_CONDITION)
        {
            if (Condition_update(&scriptedActions.actions[i].condition, ctx, screenData))
            {
                TE_Logf("SCRIPTED_ACTION", "Condition met, proceeding plot to %d", scriptedActions.actions[i].setPlotIndex);
                nextPlotIndex = scriptedActions.actions[i].setPlotIndex;
            }
            continue;
        }

        if (scriptedActions.actions[i].actionType == SCRIPTED_ACTION_TYPE_SET_NPC_TARGET)
        {
            Enemies_setTarget(scriptedActions.actions[i].id, scriptedActions.actions[i].x, scriptedActions.actions[i].y);
            continue;
        }
    }

    scriptedActions.currentPlotIndex = nextPlotIndex;
}


typedef struct Scene
{
    uint8_t id;
    void (*initFn)();
    void (*updateFn)(RuntimeContext *ctx, TE_Img *screenData);
} Scene;

static void (*_sceneUpdateFn)(RuntimeContext *ctx, TE_Img *screenData);

static void DrawTextBlock(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, const char *text)
{
    TE_Font font = GameAssets_getFont(0);
    TE_Img_fillRect(screenData, x, y, width, height, DB32Colors[21], (TE_ImgOpState){
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

static void DrawSpeechBubble(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, int16_t arrowX, int16_t arrowY, const char *text)
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

static void Scene_1_Update(RuntimeContext *ctx, TE_Img *screenData)
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

    Environment_addTreeGroup(24, 30, 1232, 5, 25);
    Environment_addTreeGroup(114, 30, 122, 5, 25);
    Environment_addTreeGroup(114, 125, 1252, 5, 25);
    Environment_addTreeGroup(24, 124, 99, 5, 20);
    // Environment_addTreeGroup(64, 84, 199, 3, 20);
}

static void Scene_2_init()
{
    Environment_addBushGroup(112, 90, 1232, 5, 10);

    Environment_addTreeGroup(24, 120, 122, 5, 25);
    Environment_addTreeGroup(104, 10, 1522, 5, 35);
    Environment_addTreeGroup(54, 20, 1622, 5, 20);
    Environment_addTreeGroup(10, 15, 522, 5, 25);
    Environment_addTree(118, 125, 5122);

    player.x = -5;
    playerCharacter.x = player.x;
    Enemies_init();
    Enemies_spawn(1, 1, 102,66);
    Enemies_spawn(2, 1, 77,42);
    Enemies_spawn(3, 2, 96,43);

    Enemies_setItem(1, ITEM_PIKE, 0);
    Enemies_setItem(2, ITEM_PIKE, 0);

    ScriptedAction_addProceedPlotCondition(0, 0, 1, (Condition){
        .type = CONDITION_TYPE_PLAYER_IN_RECT,
        .x = 40,
        .y = 0,
        .width = 20,
        .height = 128,
    });

    uint8_t step = 1;
    ScriptedAction_addPlayerControlsEnabled(step, step, 0);
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
    ScriptedAction_setNPCTarget(step, step, 1, 25, 70);
    ScriptedAction_setNPCTarget(step, step, 2, 25, 55);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "You have no right, this is my loot!", 0, 8, 86, 112, 38, 0, 10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
    step++;

    ScriptedAction_addSpeechBubble(step, step, "As the tax collector of Nottingham,", 3, 8, 4, 112, 30, 0, -10);
    ScriptedAction_addProceedPlotCondition(step, step, step + 1, (Condition){ .type = CONDITION_TYPE_PRESS_NEXT });
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
static void Scene_2_Update(RuntimeContext *ctx, TE_Img *screenData)
{
    // Update scene 2
    // DrawTextBlock(screenData, 10, 10, 108, 30, "Scene 2");

    int16_t castleX = 108;

    DrawTower(screenData, castleX+10, 64, 120);

    int16_t cartX = player.x - 20;
    int16_t cartY = 78 - (int16_t)(sin((cartX * 1.2f+12.0f) * 0.05f) * 5.0f + cartX * 0.2f);
    int16_t cartYAnchor = 80 - (int16_t)(sin(((cartX+6.0f) * 1.2f+12.0f) * 0.05f) * 5.0f + (cartX+6.0f) * 0.2f);

    player.y = 74 - (int16_t)(sin((player.x * 1.2f+12.0f) * 0.05f) * 5.0f + player.x * 0.2f);
    player.aimY = 1;
    player.dirY = 1;
    player.dy = 1;
    playerCharacter.y = player.y;
    playerCharacter.dirY = 1;

    Cart_draw(screenData, cartX, cartY,1, ctx);

    TE_Img_line(screenData, cartX+8, cartYAnchor, cartX+20, player.y + 4, DB32Colors[3], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });
    TE_Img_line(screenData, cartX+8, cartYAnchor + 1, cartX+20, player.y + 5, DB32Colors[2], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });


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
    
    if (player.x > 23)
    {
        Enemies_setTarget(3, 80,50);
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
        DrawSpeechBubble(screenData, 5,98, 118, 28, player.x, player.y + 18, text);
    }

}

static const Scene scenes[] = {
    { .id = 1, .initFn = Scene_1_init, .updateFn = Scene_1_Update },
    { .id = 2, .initFn = Scene_2_init, .updateFn = Scene_2_Update },
    {0}
};

static void NoSceneUpdate(RuntimeContext *ctx, TE_Img *screenData)
{
    // Do nothing
    DrawTextBlock(screenData, 10, 10, 108, 30, "No scene loaded");
}

void Scene_init(uint8_t sceneId)
{
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
    _sceneUpdateFn(ctx, screen);
    ScriptedAction_update(ctx, screen);
}