#include "game_battle.h"
#include "game_scenes.h"
#include "TE_Font.h"
#include "TE_math.h"

#define BATTLEMENU_LAYER 200

BattleMenuEntry BattleMenuEntry_fromAction(BattleAction *action)
{
    BattleMenuEntry entry = {
        .menuText = action->name,
        .action = action,
    };
    char *columnText = Scene_malloc(12);
    entry.columnText = columnText;
    TE_Font_concat(columnText, TE_StrFmt("%d ", action->actionPointCosts));
    TE_Font_concat(columnText, TX_SPRITE(SPRITE_HOURGLASS_6, 2, 2));
    return entry;
}

void BattleState_updateActiveActions(RuntimeContext *ctx, TE_Img *screen, BattleState *battleState)
{
    for (int i=0;i<BATTLESTATE_MAX_ENTITIES;i++)
    {
        BattleAction *action = battleState->activeActions[i];
        if (!action)
        {
            continue;
        }

        // LOG("Active action %s", action->name);
        uint8_t actionState = action->onActive(ctx, screen, battleState, action, &battleState->entities[i]);

        if (actionState == BATTLEACTION_ONACTIVE_DONE)
        {
            LOG("Action %s of entity %d done at %d", action->name, i, battleState->actionCounter);
            battleState->activeActions[i] = 0;
        }
    }
}

int16_t BattleMenuWindow_getScrollListOffset(BattleMenuWindow *window, BattleMenu *battleMenu)
{
    int16_t actionScrollListOffset = max_s16(0, min_s16(battleMenu->selectedActionY + 8 - 2 * window->lineHeight, window->h - window->lineHeight * 2 - 7));
    return actionScrollListOffset;
}

void BattleMenuWindow_update(RuntimeContext *ctx, TE_Img *screen, BattleMenuWindow* window, BattleMenu *battleMenu)
{
    TE_Font font = GameAssets_getFont(FONT_MEDIUM);
    int16_t x = window->x;
    int16_t y = window->y;
    int16_t h = window->h;
    uint8_t lineHeight = window->lineHeight;
    float dy = battleMenu->selectedAction * lineHeight - battleMenu->selectedActionY;
    battleMenu->selectedActionY += ctx->deltaTime * 10.0f * dy;

    int16_t actionScrollListOffset = BattleMenuWindow_getScrollListOffset(window, battleMenu);
    TE_ImgOpState actionStateText = (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = BATTLEMENU_LAYER + 1,
        .scissorX = max_s16(0,window->x),
        .scissorY = 0,
        .scissorHeight = lineHeight
    };
    
    for (int i=0; i < battleMenu->entriesCount; i++)
    {
        BattleMenuEntry *action = &battleMenu->entries[i];
        int16_t actionY = i * lineHeight - actionScrollListOffset;
        if (actionY < -lineHeight || actionY >= h)
        {
            continue;
        }

        if (i == battleMenu->selectedAction)
        {
            action->time += ctx->deltaTime;
            if (action->textSize.x > window->divX - window->x)
            {
                int16_t scrollWidth = action->textSize.x - (window->divX - window->x) + 20;
                float span = 0.01f * scrollWidth;
                float pingpong = 1.0f - (cosf(action->time / span) * 0.5f + 0.5f); //1.0f - fabsf(0.5f - fmodf(action->time, span) / span) * 2.0f;
                
                action->textScrollX = (int16_t)(pingpong * scrollWidth);
            }
        }
        else
        {
            action->time = 0;
            
            action->textScrollX = fmaxf(0, action->textScrollX * 0.95f - ctx->deltaTime * 20);
        }

        int16_t actionX = x + 8 - action->textScrollX;
        actionStateText.scissorX = x + 8;
        actionStateText.scissorY = max_s16(0, actionY);
        actionStateText.scissorHeight = max_s16(0, min_s16(lineHeight + 3, h + y - actionY - 1));
        if (actionStateText.scissorHeight == 0)
        {
            continue;
        }
        actionStateText.scissorWidth = window->divX - max_s16(0,window->x) - 16;
        TE_Vector2_s16 textSize = TE_Font_drawTextBox(screen, &font, actionX, actionY, 255, h, -1, 0, action->menuText, 0.0f, 0.0f, 0xffffffff, actionStateText);
        action->textSize = textSize;
        actionStateText.scissorX = window->divX;
        actionStateText.scissorWidth = window->divX2 - window->divX;
        if (action->columnText)
            TE_Font_drawTextBox(screen, &font, 83 + x, actionY, 40, h, -1, 0, action->columnText, 0.0f, 0.0f, 0xffffffff, actionStateText);
    }

    // selection highlight
    int16_t selectedY = y + battleMenu->selectedActionY + 2 - actionScrollListOffset;
    TE_Img_fillRect(screen, x + 1, selectedY, window->divX2 - 1, lineHeight + 1, window->selectedColor, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = BATTLEMENU_LAYER,
        .zAlphaBlend = 1,
    });
    TE_Img_fillTriangle(screen, x + 1, selectedY, x + 6, selectedY + lineHeight / 2, x + 1, selectedY + lineHeight, 0xaa0099ff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = BATTLEMENU_LAYER,
        .zAlphaBlend = 1,
    });
    TE_Img_fillTriangle(screen, window->divX - 2, selectedY, window->divX - 7, selectedY + lineHeight / 2, window->divX - 2, selectedY + lineHeight, 0xaa0099ff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = BATTLEMENU_LAYER,
        .zAlphaBlend = 1,
    });

    if (ctx->inputUp && !ctx->prevInputUp && !Menu_isActive())
    {
        battleMenu->selectedAction--;
        if (battleMenu->selectedAction < 0)
        {
            battleMenu->selectedAction = 0;
        }
    }
    if (ctx->inputDown && !ctx->prevInputDown && !Menu_isActive())
    {
        battleMenu->selectedAction++;
        if (battleMenu->selectedAction >= battleMenu->entriesCount)
        {
            battleMenu->selectedAction = battleMenu->entriesCount - 1;
        }
    }
}