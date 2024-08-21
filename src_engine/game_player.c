#include "game_player.h"
#include "game_character.h"
#include "game_projectile.h"
#include "TE_Image.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

void Player_update(Player *player, Character *playerCharacter, RuntimeContext *ctx, TE_Img *img)
{
    if (ctx->inputUp || ctx->inputDown)
    {
        player->dirX = player->dirY = 0;
    }

    if (ctx->inputUp)
    {
        player->dy = -1;
        player->dirY = -1;
    }
    else if (ctx->inputDown)
    {
        player->dy = 1;
        player->dirY = 1;
    }
    else
    {
        player->dy = 0;
    }

    if (ctx->inputLeft)
    {
        player->dx = -1;
        player->dirX = -1;
    }
    else if (ctx->inputRight)
    {
        player->dx = 1;
        player->dirX = 1;
    }
    else
    {
        player->dx = 0;
    }

    int sqLen = player->dx * player->dx + player->dy * player->dy;
    float multiplier = sqLen == 2 ? 0.70710678118f * 16.0f : 16.0f;
    if (sqLen == 2)
    {
        // float frac = player->x - floorf(player->x);
        // player->y = floorf(player->y) + frac;
        // printf_s("Diagonal movement %f %f\n", player->x, player->y);
    }

    player->x += player->dx * ctx->deltaTime * multiplier;
    player->y += player->dy * ctx->deltaTime * multiplier;


    float tdx = player->x - playerCharacter->x;
    float tdy = player->y - playerCharacter->y;
    if (tdx == 0.0f && tdy == 0.0f)
    {
        tdy = -1.0f;
    }
    #define SHOOT_COOLDOWN (0.8f)
    if (ctx->inputA)
    {
        playerCharacter->isAiming = 1;
        playerCharacter->itemRightHand = -2;
        playerCharacter->shootCooldown += ctx->deltaTime;
        float percent = playerCharacter->shootCooldown / SHOOT_COOLDOWN + 1.0f;
        uint32_t color = 0x44000088;
        if (percent > 1.0f) 
        {
            percent = 1.0f;
            color = ctx->frameCount / 4 % 2 == 0 ? 0xff0000ff : 0xffffffff;
        }
        float len = percent * 16.0f;
        float sdx = tdx;
        float sdy = tdy;
        float sdLen = sqrtf(sdx * sdx + sdy * sdy);
        sdx /= sdLen;
        sdy /= sdLen;

        int16_t x1 = (int)player->x, y1 = (int)player->y + 5;
        int16_t x2 = (int)player->x + sdx * 16.0f - sdy * 8.0f * (1.0f - percent);
        int16_t y2 = (int)player->y + sdy * 16.0f + sdx * 8.0f * (1.0f - percent) + 5;
        int16_t x3 = (int)player->x + sdx * 16.0f + sdy * 8.0f * (1.0f - percent);
        int16_t y3 = (int)player->y + sdy * 16.0f - sdx * 8.0f * (1.0f - percent) + 5;

        if (percent < 1.0f)
        {
            TE_Img_fillTriangle(img, x1, y1, x2, y2, x3, y3, color, (TE_ImgOpState) {
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
            });
        }
        else
        {
            TE_Img_line(img, x1, y1, x2, y2, color, (TE_ImgOpState) {
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = 200,
            });
        }
        // TE_Img_line(img, (int)player.x, (int)player.y + 5, (int)player.x + shootDx * len, (int)player.y + shootDy * len + 5, 
        //     color, (TE_ImgOpState) {
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        // });
        // TE_Img_line(img, (int)player.x, (int)player.y + 5, 
        //     (int)player.x + shootDx * 16.0f - shootDy * 8.0f * (1.0f - percent), 
        //     (int)player.y + shootDy * 16.0f + shootDx * 8.0f * (1.0f - percent)+ 5, 
        //     color, (TE_ImgOpState) {
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        // });
        // TE_Img_line(img, (int)player.x, (int)player.y + 5, 
        //     (int)player.x + shootDx * 16.0f + shootDy * 8.0f * (1.0f - percent), 
        //     (int)player.y + shootDy * 16.0f - shootDx * 8.0f * (1.0f - percent)+ 5, 
        //     color, (TE_ImgOpState) {
        //     .zCompareMode = Z_COMPARE_ALWAYS,
        //     .zValue = 200,
        // });
    }
    else if (playerCharacter->isAiming && playerCharacter->shootCooldown < 0.0f)
    {
        playerCharacter->isAiming = 0;
        playerCharacter->itemRightHand = -1;
        playerCharacter->shootCooldown = -SHOOT_COOLDOWN;
    }
    else if (playerCharacter->isAiming)
    {
        playerCharacter->isAiming = 0;
        playerCharacter->itemRightHand = -1;
        
        playerCharacter->shootCooldown = -SHOOT_COOLDOWN;
        float len = sqrtf(tdx * tdx + tdy * tdy);
        tdx /= len;
        tdy /= len;
        Projectile_spawn(player->x, player->y + 5, tdx * 128.0f, tdy * 128.0f, 0xff0000ff);
    }
    else
    {
        playerCharacter->shootCooldown = -SHOOT_COOLDOWN;
    }


    Character_update(playerCharacter, ctx, img, player->x, player->y, player->dirX, player->dirY);

    TE_Img_fillRect(img, 0, 0, 128, 11, DB32Colors[1], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    
    int currentHealth = player->health;
    for (int i=0; i < player->maxHealth / 2; i++)
    {
        int x = 8 + i * 8;
        int y = 2;
        int srcX = i == currentHealth / 2 && currentHealth % 2 == 0 ? 9 : 0;
        if (i > currentHealth / 2)
        {
            srcX = 18;
        }
        TE_Img_blitEx(img, &atlasImg, x, y, srcX, 112, 9, 8, (BlitEx) {
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

    TE_Img_blitEx(img, &atlasImg, 0, 0, 2, 64, 11, 11, (BlitEx) {
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