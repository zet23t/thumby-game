#include "game_menu.h"
#include "game.h"
#include "TE_Font.h"
#include "fnt_myfont.h"
#include "fnt_tiny.h"
#include "fnt_medium.h"
#include <math.h>

typedef struct GameMenu
{
    int8_t isActive:1;
    int8_t isTriggered:1;
} GameMenu;

static GameMenu gameMenu;

void Menu_update(RuntimeContext *ctx, TE_Img* img)
{
    if (ctx->inputMenu)
    {
        if (!gameMenu.isTriggered)
            gameMenu.isActive = !gameMenu.isActive;
        gameMenu.isTriggered = 1;
    }
    else 
    {
        gameMenu.isTriggered = 0;
    }

    if (!gameMenu.isActive)
    {
        return;
    }
    TE_Img myFontImg = {
        .p2width = fnt_myfont_p2width,
        .p2height = fnt_myfont_p2height,
        .data = (uint32_t*) fnt_myfont_data,
    };
    
    TE_Img tinyImg = {
        .p2width = fnt_tiny_p2width,
        .p2height = fnt_tiny_p2height,
        .data = (uint32_t*) fnt_tiny_data,
    };

    TE_Img mediumImg = {
        .p2width = fnt_medium_p2width,
        .p2height = fnt_medium_p2height,
        .data = (uint32_t*) fnt_medium_data,
    };

    TE_Font myfont = {
        .atlas = &myFontImg,
        .glyphCount = fnt_myfont_glyph_count,
        .glyphValues = fnt_myfont_glyphs_values,
        .rectXs = fnt_myfont_glyphs_rects_x,
        .rectYs = fnt_myfont_glyphs_rects_y,
        .rectWidths = fnt_myfont_glyphs_rects_width,
        .rectHeights = fnt_myfont_glyphs_rects_height,
    };

    TE_Font tinyfont = {
        .atlas = &tinyImg,
        .glyphCount = fnt_tiny_glyph_count,
        .glyphValues = fnt_tiny_glyphs_values,
        .rectXs = fnt_tiny_glyphs_rects_x,
        .rectYs = fnt_tiny_glyphs_rects_y,
        .rectWidths = fnt_tiny_glyphs_rects_width,
        .rectHeights = fnt_tiny_glyphs_rects_height,
    };

    TE_Font mediumfont = {
        .atlas = &mediumImg,
        .glyphCount = fnt_medium_glyph_count,
        .glyphValues = fnt_medium_glyphs_values,
        .rectXs = fnt_medium_glyphs_rects_x,
        .rectYs = fnt_medium_glyphs_rects_y,
        .rectWidths = fnt_medium_glyphs_rects_width,
        .rectHeights = fnt_medium_glyphs_rects_height,
    };

    int menuX = 16;
    int menuY = 20;

    TE_Img_drawPatch9(img, &atlasImg, menuX, menuY, 127-menuX * 2, 127-menuY * 2, 56, 104, 8, 8, (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_ALWAYS,
            .zValue = 255,
        }
    });

    #define TITLE "Robin of Lootly"
    int titleWidth = TE_Font_getWidth(&myfont, TITLE, -1);
    int titleX = menuX + ((127-menuX * 2) - titleWidth) / 2;
    float alignX = cosf(ctx->time * 1.25f) * 0.5f + 0.5f;
    float alignY = sinf(ctx->time * 2.5f) * 0.5f + 0.5f;
    TE_Font_drawTextBox(img, &mediumfont, menuX + 4, menuY + 4, 127-(menuX + 4)*2 - 2, 64, -1, -4, TITLE, alignX, 0.0f, 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });

    TE_Font_drawTextBox(img, &mediumfont, menuX + 4, menuY + 18, 127-(menuX + 4)*2 - 2, 64, -1, -4, "This is some longer text test with line breaks in between", alignX, alignY, 0xffffffff, (TE_ImgOpState)
    {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}