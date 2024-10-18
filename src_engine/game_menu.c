#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "game_menu.h"
#include "game.h"
#include "TE_Font.h"
#include "fnt_myfont.h"
#include "fnt_tiny.h"
#include "fnt_medium.h"
#include "TE_math.h"
#include "game_renderobjects.h"

typedef struct GameMenu
{
    int8_t isActive:1;
    int8_t isTriggered:1;
    float openProgress;
} GameMenu;

static GameMenu gameMenu;

uint8_t Menu_isActive()
{
    return gameMenu.isActive;
}

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

    float target = gameMenu.isActive ? 1.0f : 0.0f;
    gameMenu.openProgress = fMoveTowards(gameMenu.openProgress, target, ctx->deltaTime * 2.0f);
    if (!gameMenu.isActive && gameMenu.openProgress <= 0.001f)
    {
        return;
    }
    // TE_Img myFontImg = {
    //     .p2width = fnt_myfont_p2width,
    //     .p2height = fnt_myfont_p2height,
    //     .data = (uint32_t*) fnt_myfont_data,
    // };
    
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

    // TE_Font myfont = {
    //     .atlas = &myFontImg,
    //     .glyphCount = fnt_myfont_glyph_count,
    //     .glyphValues = fnt_myfont_glyphs_values,
    //     .rectXs = fnt_myfont_glyphs_rects_x,
    //     .rectYs = fnt_myfont_glyphs_rects_y,
    //     .rectWidths = fnt_myfont_glyphs_rects_width,
    //     .rectHeights = fnt_myfont_glyphs_rects_height,
    // };

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
    int menuY = fLerpClamped(-8, 20, gameMenu.openProgress * 8.0f);

    int16_t menuWidth = 127 - menuX * 2;
    int16_t menuHeight = 127 - menuY * 2;
    menuHeight = (int16_t)(menuHeight * fTweenElasticOut(gameMenu.openProgress * 1.125f - 0.125f));

    for (int i=0;i<2;i++)
    {
        TE_Img_line(img, 64, i, menuWidth + menuX, menuY + i, DB32Colors[3 - i], (TE_ImgOpState) {
            .zCompareMode = Z_COMPARE_ALWAYS,
            .zValue = 255,
        });
        TE_Img_line(img, 64, i, menuX - 4, menuY + i, DB32Colors[3 - i], (TE_ImgOpState) {
            .zCompareMode = Z_COMPARE_ALWAYS,
            .zValue = 255,
        });
    }

    TE_Img_drawPatch9(img, &atlasImg, menuX, menuY, menuWidth, menuHeight, 56, 104, 8, 8, (BlitEx) {
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
    float alignX = 0.5f;// cosf(ctx->time * 1.25f) * 0.5f + 0.5f;
    float alignY = 0.0f; //sinf(ctx->time * 2.5f) * 0.5f + 0.5f;
    int clipHeight = menuHeight + (menuY < 0 ? menuY : 0);
    if (clipHeight < 0)
    {
        clipHeight = 0;
    }
    TE_Font_drawTextBox(img, &mediumfont, menuX + 4, menuY + 4, 127-(menuX + 4)*2 - 2, 64, -1, -4, TITLE, alignX, 0.0f, 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
        .scissorX = menuX,
        .scissorY = menuY < 0 ? 0 : menuY,
        .scissorWidth = menuWidth,
        .scissorHeight = clipHeight,
    });

    if (ctx->inputA && !ctx->prevInputA)
    {
        ctx->drawStats = !ctx->drawStats;
    }

    if (ctx->inputB && !ctx->prevInputB)
    {
        if (ctx->sfxChannelStatus[0].flagIsPlaying)
        {
            LOG("Menu: Pausing music");
            ctx->outSfxInstructions[0] = (SFXInstruction)
            {
                .type = SFXINSTRUCTION_TYPE_PAUSE
            };
        }
        else
        {
            LOG("Menu: Playing music");
            static int musicId = 0;
            ctx->outSfxInstructions[0] = (SFXInstruction)
            {
                .type = SFXINSTRUCTION_TYPE_PLAY,
                .id = musicId++%5,
                .updateMask = SFXINSTRUCTION_UPDATE_MASK_VOLUME,
                .volume = 150
            };
        }
    }

    char info[128];
    sprintf(info, "Draw stats (A): %s - Play Music (B) - Music by ModArchive.org: "
        "4mat-Wizardry; 2_core; 1987-tune; greensleeves_thx; nitabrowski", 
        ctx->drawStats ? "yes" : "no");
    

    TE_Font_drawTextBox(img, &tinyfont, menuX + 4, menuY + 18, 127-(menuX + 4)*2 - 2, 64, -1, -4, 
        info, alignX, alignY, 0xffffffff, (TE_ImgOpState)
    {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
        .scissorX = menuX,
        .scissorY = menuY < 0 ? 0 : menuY,
        .scissorWidth = menuWidth,
        .scissorHeight = clipHeight,
    });

    // barrels
    TE_Img_drawPatch9(img, &atlasImg, menuX-5, menuY-2, menuWidth+8, 12, 80, 100, 6, 4, (BlitEx) {
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

    TE_Img_drawPatch9(img, &atlasImg, menuX-5, menuY-7+menuHeight, menuWidth+8, 12, 80, 100, 6, 4, (BlitEx) {
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
}