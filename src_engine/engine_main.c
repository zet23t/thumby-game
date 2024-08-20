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

uint32_t DB32Colors[] = {
    0xFF000000, 0xFF342022, 0xFF3C2845, 0xFF313966, 0xFF3B568F, 0xFF2671DF, 0xFF66A0D9, 0xFF9AC3EE,
    0xFF36F2FB, 0xFF50E599, 0xFF30BE6A, 0xFF6E9437, 0xFF2F694B, 0xFF244B52, 0xFF393C32, 0xFF743F3F,
    0xFF826030, 0xFFE16E5B, 0xFFFF9B63, 0xFFE4CD5F, 0xFFFCDBCB, 0xFFFFFFFF, 0xFFB7AD9B, 0xFF877E84,
    0xFF6A6A69, 0xFF525659, 0xFF8A4276, 0xFF3232AC, 0xFF6357D9, 0xFFBA7BD7, 0xFF4A978F, 0xFF306F8A,
    0xFF36535E, 0xFF48687D, 0xFF3C7EA0, 0xFFC7C3C2, 0xFFE0E0E0,
};

DLL_EXPORT void init()
{
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

    enemyCharacters[0] = (Character)
    {
        .srcHeadFront = { .x = 48, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48, .y = 64, .width = 8, .height = 6 },
    };

    Enemies_spawn(0, 28, 42);
    Enemies_spawn(0, 44, 28);

    
}

DLL_EXPORT void update(RuntimeContext *ctx)
{
    if (ctx->inputUp || ctx->inputDown)
    {
        player.dirX = player.dirY = 0;
    }

    if (ctx->inputUp)
    {
        player.dy = -1;
        player.dirY = -1;
    }
    else if (ctx->inputDown)
    {
        player.dy = 1;
        player.dirY = 1;
    }
    else
    {
        player.dy = 0;
    }

    if (ctx->inputLeft)
    {
        player.dx = -1;
        player.dirX = -1;
    }
    else if (ctx->inputRight)
    {
        player.dx = 1;
        player.dirX = 1;
    }
    else
    {
        player.dx = 0;
    }

    int sqLen = player.dx * player.dx + player.dy * player.dy;
    float multiplier = sqLen == 2 ? 0.70710678118f * 16.0f : 16.0f;
    if (sqLen == 2)
    {
        float frac = player.x - (int)player.x;
        player.y = (int)player.y + frac;
    }

    player.x += player.dx * ctx->deltaTime * multiplier;
    player.y += player.dy * ctx->deltaTime * multiplier;

    TE_Img img = {
        .p2width = 7,
        .p2height = 7,
        .data = ctx->screenData,
    };

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

    
    TE_Img_clear(&img, DB32Colors[13], 0);
    TE_randSetSeed(3294);

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
    DrawTree(&img, 30,50);
    // DrawTree(&img, 35,50);
    // DrawTree(&img, 25,30);
    DrawTree(&img, 60,50);
    DrawTree(&img, 90,50);
    DrawTree(&img, 35,90);
    DrawTree(&img, 65,110);
    DrawTree(&img, 90,80);
    DrawTree(&img, 95,80);
    DrawTree(&img, 95,90);
    // DrawTree(&img, 30,80);
    // DrawTree(&img, 60,80);
    // DrawTree(&img, 90,80);

    Enemies_update(ctx, &img);

    Projectiles_update(projectiles, ctx, &img);
    int shootDx = player.dx;
    int shootDy = player.dy;
    if (shootDx == 0 && shootDy == 0)
    {
        shootDx = player.dirX;
        shootDy = player.dirY;
    }
    #define SHOOT_COOLDOWN (0.8f)
    if (ctx->inputA)
    {
        playerCharacter.isAiming = 1;
        playerCharacter.itemRightHand = -2;
        playerCharacter.shootCooldown += ctx->deltaTime;
        float percent = playerCharacter.shootCooldown / SHOOT_COOLDOWN + 1.0f;
        uint32_t color = 0x44000088;
        if (percent > 1.0f) 
        {
            percent = 1.0f;
            color = ctx->frameCount / 4 % 2 == 0 ? 0xff0000ff : 0xffffffff;
        }
        float len = percent * 16.0f;
        float sdx = shootDx;
        float sdy = shootDy;
        float sdLen = sqrtf(sdx * sdx + sdy * sdy);
        sdx /= sdLen;
        sdy /= sdLen;

        int16_t x1 = (int)player.x, y1 = (int)player.y + 5;
        int16_t x2 = (int)player.x + sdx * 16.0f - sdy * 8.0f * (1.0f - percent);
        int16_t y2 = (int)player.y + sdy * 16.0f + sdx * 8.0f * (1.0f - percent) + 5;
        int16_t x3 = (int)player.x + sdx * 16.0f + sdy * 8.0f * (1.0f - percent);
        int16_t y3 = (int)player.y + sdy * 16.0f - sdx * 8.0f * (1.0f - percent) + 5;

        if (percent < 1.0f)
        {
            TE_Img_fillTriangle(&img, x1, y1, x2, y2, x3, y3, color, (TE_ImgOpState) {
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
            });
        }
        else
        {
            TE_Img_line(&img, x1, y1, x2, y2, color, (TE_ImgOpState) {
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
            });
        }
        // TE_Img_line(&img, (int)player.x, (int)player.y + 5, (int)player.x + shootDx * len, (int)player.y + shootDy * len + 5, 
        //     color, (TE_ImgOpState) {
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        // });
        // TE_Img_line(&img, (int)player.x, (int)player.y + 5, 
        //     (int)player.x + shootDx * 16.0f - shootDy * 8.0f * (1.0f - percent), 
        //     (int)player.y + shootDy * 16.0f + shootDx * 8.0f * (1.0f - percent)+ 5, 
        //     color, (TE_ImgOpState) {
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        // });
        // TE_Img_line(&img, (int)player.x, (int)player.y + 5, 
        //     (int)player.x + shootDx * 16.0f + shootDy * 8.0f * (1.0f - percent), 
        //     (int)player.y + shootDy * 16.0f - shootDx * 8.0f * (1.0f - percent)+ 5, 
        //     color, (TE_ImgOpState) {
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        // });
    }
    else if (playerCharacter.isAiming && playerCharacter.shootCooldown < 0.0f)
    {
        playerCharacter.isAiming = 0;
        playerCharacter.itemRightHand = -1;
        playerCharacter.shootCooldown = -SHOOT_COOLDOWN;
    }
    else if (playerCharacter.isAiming)
    {
        playerCharacter.isAiming = 0;
        playerCharacter.itemRightHand = -1;
        
        playerCharacter.shootCooldown = -SHOOT_COOLDOWN;
        Projectile_spawn(player.x, player.y + 5, shootDx * 128.0f, shootDy * 128.0f, 0xff0000ff);
    }
    else
    {
        playerCharacter.shootCooldown = -SHOOT_COOLDOWN;
    }


    Character_update(&playerCharacter, ctx, &img, player.x, player.y, player.dirX, player.dirY);
    
    

    TE_Font_drawText(&img, &myfont, 2, 2, -1, "Sherwood Forest", 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 100,
    });

    char text[64];
    sprintf(text, "FPS: %d", (int)(1.0f / ctx->deltaTime));
    TE_Font_drawText(&img, &tinyfont, 2, 112, -1, text, 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 100,
    });
}