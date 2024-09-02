#include "engine_main.h"
#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include <atlas.h>
#include <fnt_myfont.h>
#include <fnt_tiny.h>

#include "TE_Image.h"
#include "TE_rand.h"
#include "TE_Font.h"
#include "game.h"
#include "game_projectile.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_player.h"
#include "game_menu.h"
#include "game_environment.h"
#include "game_scenes.h"
#include "stdarg.h"

uint32_t DB32Colors[] = {
    0xFF000000, 0xFF342022, 0xFF3C2845, 0xFF313966, 0xFF3B568F, 0xFF2671DF, 0xFF66A0D9, 0xFF9AC3EE,
    0xFF36F2FB, 0xFF50E599, 0xFF30BE6A, 0xFF6E9437, 0xFF2F694B, 0xFF244B52, 0xFF393C32, 0xFF743F3F,
    0xFF826030, 0xFFE16E5B, 0xFFFF9B63, 0xFFE4CD5F, 0xFFFCDBCB, 0xFFFFFFFF, 0xFFB7AD9B, 0xFF877E84,
    0xFF6A6A69, 0xFF525659, 0xFF8A4276, 0xFF3232AC, 0xFF6357D9, 0xFFBA7BD7, 0xFF4A978F, 0xFF306F8A,
    0xFF36535E, 0xFF48687D, 0xFF3C7EA0, 0xFFC7C3C2, 0xFFE0E0E0,
};

void TE_Logf(const char *tag, const char *fmt, ...)
{
    printf("[%s] ", tag);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}


DLL_EXPORT void init()
{
    TE_Logf(LOG_TAG_SYSTEM, "Initializing");
    atlasImg = (TE_Img) {
        .p2width = atlas_p2width,
        .p2height = atlas_p2height,
        .data = (uint32_t*) atlas_data,
    };

    items[0] = (Item) {
        .pivotX = 2,
        .pivotY = 4,
        .src = { .x = 0, .y = 96, .width = 6, .height = 13 },
    };
    items[1] = (Item) {
        .pivotX = 6,
        .pivotY = 4,
        .src = { .x = 7, .y = 96, .width = 9, .height = 13 },
    };
    
    items[ITEM_PIKE - 1] = (Item) {
        .pivotX = 1,
        .pivotY = 8,
        .src = { .x = 64, .y = 80, .width = 8, .height = 16 },
    };

    items[ITEM_STAFF - 1] = (Item) {
        .pivotX = 0,
        .pivotY = 8,
        .src = { .x = 16, .y = 96, .width = 3, .height = 16 },
    };

    playerCharacter = (Character)
    {
        .x = player.x,
        .y = player.y,
        .itemRightHand = -1,
        .srcHeadFront = { .x = 0, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 0, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40, .y = 64, .width = 8, .height = 6 },
    };

    // Henchman
    characters[1] = (Character)
    {
        .srcHeadFront = { .x = 48, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48, .y = 64, .width = 8, .height = 6 },
    };
    // Dinglewort
    characters[2] = (Character)
    {
        .srcHeadFront = { .x = 48 * 2, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48 * 2, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48 * 2, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48 * 2, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48 * 2, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48 * 2, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48 * 2, .y = 64, .width = 8, .height = 6 },
    };
    // bandit 1
    characters[3] = (Character)
    {
        .srcHeadFront = { .x = 48 * 3, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48 * 3, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48 * 3, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48 * 3, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48 * 3, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48 * 3, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48 * 3, .y = 64, .width = 8, .height = 6 },
    };
    // bandit 2
    characters[4] = (Character)
    {
        .srcHeadFront = { .x = 160, .y = 80, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 176, .y = 80, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48 * 3, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48 * 3, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48 * 3, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48 * 3, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48 * 3, .y = 64, .width = 8, .height = 6 },
    };

    Scene_init(3);
    // Environment_addTree(30,50, 12343050);
    // // Environment_addTree(35,50, 12343550);
    // // Environment_addTree(25,30, 12342530);
    // Environment_addTree(60,50, 12346050);
    // Environment_addTree(90,50, 12349050);
    // Environment_addTree(35,90, 12343590);
    // Environment_addTree(65,110, 12346511);
    // Environment_addTree(90,80, 12349080);
    // Environment_addTree(95,80, 12349580);
    // Environment_addTree(95,90, 12349590);
    // // Environment_addTree(30,80, 12343080);
    // // Environment_addTree(60,80, 12346080);
    // // Environment_addTree(90,80, 12349080);
}

DLL_EXPORT void update(RuntimeContext *ctx)
{
    TE_Img img = {
        .p2width = 7,
        .p2height = 7,
        .data = ctx->screenData,
    };
    
    TE_Img tinyImg = {
        .p2width = fnt_tiny_p2width,
        .p2height = fnt_tiny_p2height,
        .data = (uint32_t*) fnt_tiny_data,
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

    
    TE_Img_clear(&img, DB32Colors[13], 0);


    TE_randSetSeed(Scene_getCurrentSceneId() * 127 + 392);
    for (int i=0;i<24;i++)
    {
        int x = TE_randRange(0, 120);
        int y = TE_randRange(0, 120);
        TE_Img_blitEx(&img, &atlasImg, x, y, 16, 0, 8, 8, (BlitEx) {
            .flipX = TE_randRange(0, 2),
            .flipY = 0,
            .rotate = 0,
            .tint = 1,
            .blendMode = TE_BLEND_ALPHAMASK,
            .tintColor = 0xff306f8a,
        });
    }
    

    Scene_update(ctx, &img);
    Enemies_update(ctx, &img);
    Projectiles_update(projectiles, ctx, &img);

    Player_update(&player, &playerCharacter, ctx, &img);
    
    Environment_update(ctx, &img);
    ScriptedAction_update(ctx, &img);
    Menu_update(ctx, &img);

    // TE_Font_drawText(&img, &myfont, 2, 2, -1, "Sherwood Forest", 0xffffffff, (TE_ImgOpState) {
    //     .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //     .zValue = 100,
    // });

    int fps = (int)roundl(1.0f / ctx->deltaTime);
    if (fps > 25)
    {
        return;
    }
    char text[64];
    sprintf(text, "FPS: %d", fps);
    TE_Font_drawText(&img, &tinyfont, 2, 112, -1, text, 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });

}