#include "game_battle.h"
#include "TE_math.h"

void BattleMenuWindow_update(RuntimeContext *ctx, TE_Img *screen, BattleMenuWindow* window, BattleMenu *battleMenu)
{
    TE_Font font = GameAssets_getFont(FONT_MEDIUM);
    int16_t x = window->x;
    int16_t y = window->y;
    int16_t h = window->h;
    uint8_t lineHeight = window->lineHeight;
    int16_t actionScrollListOffset = max_s16(0, (battleMenu->selectedAction - 2) * lineHeight);
    TE_ImgOpState actionState = (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 201,
        .scissorX = 0,
        .scissorY = 0,
        .scissorWidth = window->divX2,
        .scissorHeight = h - 2
    };
    
    for (int i=0; i < battleMenu->entriesCount; i++)
    {
        BattleMenuEntry *action = &battleMenu->entries[i];
        int16_t actionY = i * lineHeight - actionScrollListOffset;
        TE_Font_drawTextBox(screen, &font, 8 + x, actionY, 80, h, -1, -3, action->menuText, 0.0f, 0.0f, 0xffffffff, actionState);
        if (action->columnText)
            TE_Font_drawTextBox(screen, &font, 83 + x, actionY, 40, h, -1, -3, action->columnText, 0.0f, 0.0f, 0xffffffff, actionState);
    }

    // selection highlight
    int16_t selectedY = y + battleMenu->selectedAction * lineHeight + 2 - actionScrollListOffset;
    TE_Img_fillRect(screen, x + 1, selectedY, window->divX2 - 1, lineHeight + 1, window->selectedColor, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 200,
        .zAlphaBlend = 1,
    });
    TE_Img_fillTriangle(screen, x + 1, selectedY, x + 6, selectedY + lineHeight / 2, x + 1, selectedY + lineHeight, 0xaa0099ff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 200,
        .zAlphaBlend = 1,
    });
    TE_Img_fillTriangle(screen, window->divX - 2, selectedY, window->divX - 7, selectedY + lineHeight / 2, window->divX - 2, selectedY + lineHeight, 0xaa0099ff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 200,
        .zAlphaBlend = 1,
    });

    if (ctx->inputUp && !ctx->prevInputUp)
    {
        battleMenu->selectedAction--;
        if (battleMenu->selectedAction < 0)
        {
            battleMenu->selectedAction = 0;
        }
    }
    if (ctx->inputDown && !ctx->prevInputDown)
    {
        battleMenu->selectedAction++;
        if (battleMenu->selectedAction >= battleMenu->entriesCount)
        {
            battleMenu->selectedAction = battleMenu->entriesCount - 1;
        }
    }
}