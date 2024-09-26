#include "game_player.h"
#include "game_character.h"
#include "game_projectile.h"
#include "game_enemies.h"
#include "game_assets.h"
#include "game_particlesystem.h"
#include "TE_Image.h"
#include "TE_rand.h"
#include "TE_math.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

static uint8_t _playerWeaponIndex;
static uint8_t _playerInputEnabled;

void Player_setWeapon(uint8_t weaponIndex)
{
    _playerWeaponIndex = weaponIndex;
}

void Player_update(Player *player, Character *playerCharacter, RuntimeContext *ctx, TE_Img *img)
{
    if (_playerInputEnabled)
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
    }
    else
    {
        player->dx = player->dy = 0;
    }

    int sqLen = player->dx * player->dx + player->dy * player->dy;
    float multiplier = sqLen == 2 ? 0.70710678118f * 32.0f : 32.0f;
    if (sqLen == 2)
    {
        // float frac = player->x - floorf(player->x);
        // player->y = floorf(player->y) + frac;
        // printf_s("Diagonal movement %f %f\n", player->x, player->y);
    }
    // if (ctx->inputA)
    // {
    //     // multiplier *= 0.5f;
    // }
    player->x += player->dx * ctx->deltaTime * multiplier;
    player->y += player->dy * ctx->deltaTime * multiplier;

    if (player->dx || player->dy)
    {
        float ang = atan2f(player->dy, player->dx);
        float currentAng = atan2f(player->aimY, player->aimX);
        float diff = ang - currentAng;
        if (diff > M_PI)
        {
            diff -= 2.0f * M_PI;
        }
        if (diff < -M_PI)
        {
            diff += 2.0f * M_PI;
        }
        float maxDelta = ctx->deltaTime * 4.0f;
        float change = maxDelta > fabsf(diff) ? diff : (diff < 0 ? -maxDelta : maxDelta);

        float nextAng = currentAng + change;
        player->aimX = cosf(nextAng);
        player->aimY = sinf(nextAng);
    }

    float tdx = player->aimX;
    float tdy = player->aimY;
    if (tdx == 0.0f && tdy == 0.0f)
    {
        tdy = -1.0f;
    }
    playerCharacter->itemRightHand = _playerWeaponIndex;

    Item *item = &items[_playerWeaponIndex > 0 ? _playerWeaponIndex - 1 : 0];
    float coolDown = item->cooldown;

    if (ctx->inputA && 0 && _playerWeaponIndex)
    {
        playerCharacter->isAiming = 1;
        // playerCharacter->itemRightHand = -2;
        playerCharacter->shootCooldown += ctx->deltaTime;
        float percent = playerCharacter->shootCooldown / coolDown + 1.0f;
        uint32_t color = 0x44000088;
        if (percent > 1.0f) 
        {
            percent = 1.0f;
            color = ctx->frameCount / 4 % 2 == 0 ? 0xff0000ff : 0xffffffff;
        }
        // float len = percent * 16.0f;
        float sdx = tdx;
        float sdy = tdy;
        float sdLen = sqrtf(sdx * sdx + sdy * sdy);
        sdx /= sdLen;
        sdy /= sdLen;
        float range = 16.0f;
        if (item->meleeRange > 0)
        {
            range = item->meleeRange * 1.5f;
        }

        int16_t x1 = (int)playerCharacter->x, y1 = (int)playerCharacter->y + 5;
        int16_t x2 = (int)playerCharacter->x + sdx * range - sdy * range * 0.5f * (1.0f - percent);
        int16_t y2 = (int)playerCharacter->y + sdy * range + sdx * range * 0.5f * (1.0f - percent) + 5;
        int16_t x3 = (int)playerCharacter->x + sdx * range + sdy * range * 0.5f * (1.0f - percent);
        int16_t y3 = (int)playerCharacter->y + sdy * range - sdx * range * 0.5f * (1.0f - percent) + 5;

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
        // playerCharacter->itemRightHand = -1;
        playerCharacter->shootCooldown = -coolDown;
    }
    else if (playerCharacter->isAiming)
    {
        playerCharacter->isAiming = 0;
        playerCharacter->isStriking = 1;
        playerCharacter->runningAnimationTime = 0.0f;
        // playerCharacter->itemRightHand = -1;
        
        playerCharacter->shootCooldown = -coolDown;
        float len = sqrtf(tdx * tdx + tdy * tdy);
        tdx /= len;
        tdy /= len;
        if (item->meleeRange > 0)
        {
            int16_t cx, cy, cr;
            float baseX = player->x, baseY = player->y;
            Character_toBaseF(playerCharacter, &baseX, &baseY);
            int16_t hitX = (int16_t)(baseX + tdx * item->meleeRange);
            int16_t hitY = (int16_t)(baseY + tdy * item->meleeRange);
            // TE_Debug_drawPixel(hitX, hitY, 0xffffffff);
            LOG("%d %d %d", hitX, hitY, item->meleeRange);
            int hitInfo = Characters_raycastCircle(playerCharacter, hitX, hitY, item->meleeRange, &cx, &cy, &cr) - 1;
            if (player->attackQuality == 0)
            {
                ParticleSystem_spawn(PARTICLE_TYPE_SPRITE, hitX, hitY, 150, 0.0f, -15.0f, (ParticleTypeData){
                    .spriteType = {
                        .spriteId = SPRITE_TEXT_MISS,
                        .maxLife = 0.35f,
                        .color = 0xffffffff,
                    }
                });
            }
            if (player->attackQuality == 2)
            {
                ParticleSystem_spawn(PARTICLE_TYPE_SPRITE, hitX, hitY, 150, 0.0f, -15.0f, (ParticleTypeData){
                    .spriteType = {
                        .spriteId = SPRITE_TEXT_CRIT,
                        .maxLife = 0.35f,
                        .color = 0xffffffff,
                    }
                });
            }
            if (hitInfo>=0 && player->attackQuality > 0)
            {
                LOG("Hit %d %d %d %d", cx, cy, cr, TE_randGetSeed());
                playerCharacter->isHitting = 1;
                playerCharacter->isStriking = 0;
                float damage = player->attackQuality == 2 ? 1.5f : 1.0f;
                Enemy_takeDamage(&enemies[hitInfo], damage, tdx * 128.0f, tdy * 128.0f, ctx, img);

                ParticleSystem_spawn(PARTICLE_TYPE_SPRITE, 
                    cx + tdx * 15.0f, 
                    cy + tdy * 15.0f, 
                    120, tdx * 15.0f, tdy * 15.0f, (ParticleTypeData){
                    .spriteType = {
                        .spriteId = TE_randRange(SPRITE_TEXT_OUCH, SPRITE_TEXT_OOF + 1),
                        .maxLife = 0.5f * damage,
                        .color = 0xffffffff,
                    }
                });
            }
        } 
        else
        {
            Projectile_spawn(player->x, player->y + 5, tdx * 128.0f, tdy * 128.0f, 0xff0000ff);
        }
    }
    else
    {
        playerCharacter->shootCooldown = -coolDown;
    }

    float targetX = player->x;
    float targetY = player->y;
    int dirX = player->dirX;
    int dirY = player->dirY;
    if (!_playerInputEnabled)
    {
        targetX = playerCharacter->targetX;
        targetY = playerCharacter->targetY;
        dirX = playerCharacter->x < targetX ? 1 : (playerCharacter->x > targetX ? -1 : playerCharacter->dirX);
        dirY = playerCharacter->y < targetY ? 1 : (playerCharacter->y > targetY ? -1 : playerCharacter->dirY);
        // TE_Logf("DBG", "Player target %f %f : %f %f", targetX, targetY, player->dirX, player->dirY);
    }
    Character_update(playerCharacter, ctx, img, targetX, targetY, dirX, dirY);
    player->x = playerCharacter->x * .15f + player->x * .85f;
    player->y = playerCharacter->y * .15f + player->y * .85f;

    if (player->drawBar)
    {
        TE_Img_fillRect(img, 0, 0, 128, 12, DB32Colors[1], (TE_ImgOpState) {
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

    // attack & defense mini game
    uint8_t isAiming = playerCharacter->isAiming;
    uint8_t isDefending = player->defenseActionStep[0] > 0.0f;
    if (!isAiming)
    {
        player->aimTimer = 0;
    }
    if (!isDefending)
    {
        player->defTimer = 0;
    }
    
    if (!isAiming && !isDefending) return;

    int16_t uiX = floorf(playerCharacter->x + .5f) - 2;
    int16_t uiY = floorf(playerCharacter->y + .5f) + 13;
    TE_Sprite battleBar = GameAssets_getSprite(SPRITE_UI_BATTLE_BAR);
    TE_Img_blitSprite(img, battleBar, 
        uiX, uiY, (BlitEx) {
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zValue = 200,
        }
    });
    int battleBarInnerWidth = battleBar.src.width - 2;

    if (isAiming)
    {
        player->aimTimer += ctx->deltaTime;
        int attackAim = absi((int)(player->aimTimer * 10.0f) % battleBarInnerWidth * 2 - battleBarInnerWidth);
        if (attackAim >= battleBarInnerWidth - 1)
        {
            player->attackQuality = 2;
        }
        else if (attackAim > battleBarInnerWidth - 4)
        {
            player->attackQuality = 1;
        }
        else
        {
            player->attackQuality = 0;
        }
        uint32_t color = playerCharacter->shootCooldown < 0.0f ? 0x440000ff : 0xffffffff;
        TE_Img_blitSprite(img, GameAssets_getSprite(SPRITE_UI_SWORD), uiX + attackAim - battleBar.pivotX, uiY + 3, (BlitEx) {
            .blendMode = TE_BLEND_ALPHAMASK,
            .flipY = 1,
            .tint = 1,
            .tintColor = color,
            .state = {
                .zValue = 200,
                .zAlphaBlend = 1,
            }
        });
    }

    if (isDefending)
    {
        int defenseAim = absi((int)(player->defenseActionStep[0] * 7.0f) % battleBarInnerWidth * 2 - battleBarInnerWidth);
        if (defenseAim >= battleBarInnerWidth - 1)
        {
            player->defenseQuality = 2;
        }
        else if (defenseAim > battleBarInnerWidth - 4)
        {
            player->defenseQuality = 1;
        }
        else
        {
            player->defenseQuality = 0;
        }
        TE_Img_blitSprite(img, GameAssets_getSprite(SPRITE_UI_SHIELD), uiX + defenseAim - battleBar.pivotX, uiY + 3, (BlitEx) {
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zValue = 200,
            }
        });
    }
}

void Player_setInputEnabled(uint8_t enabled)
{
    _playerInputEnabled = enabled;
}