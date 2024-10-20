#include "game.h"
#include "game_environment.h"
#include "game_character.h"
#include "game_assets.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define RECTARG(r) r.x, r.y, r.width, r.height

int Character_raycastCircle(Character *character, int16_t px, int16_t py, int16_t radius, int16_t *outCenterX, int16_t *outCenterY, int16_t *outRadius)
{
    int baseX = character->x - 1;
    int baseY = character->y + 6;
    int16_t dx = px - baseX;
    int16_t dy = py - baseY;
    int16_t sqLen = dx * dx + dy * dy;
    int16_t cmpLen = radius + 4;
    if (sqLen < cmpLen * cmpLen)
    {
        *outCenterX = baseX;
        *outCenterY = baseY;
        *outRadius = 4;
        return 1;
    }
    return 0;
}

void Character_toBaseF(Character *character, float *x, float *y)
{
    *x -= 1.0f;
    *y += 6.0f;
}

void Character_fromBaseF(Character *character, float *x, float *y)
{
    *x += 1.0f;
    *y -= 6.0f;
}

void Character_drawKO(TE_Img *img, Character *character, uint8_t zOffset)
{
    int16_t x = (int16_t) floorf(character->x) - 5;
    int16_t y = (int16_t) floorf(character->y) - 3;
    uint8_t charZ = (uint8_t) character->y + 14 + zOffset;
    TE_Img_blitEx(img, &atlasImg, x, y, RECTARG(character->srcHeadBack), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 3,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ,
        }
    });
    TE_Img_blitEx(img, &atlasImg, x - 2, y - 1, RECTARG(character->srcBodyBack), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 3,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ - 1,
        }
    });

    TE_Img_blitEx(img, &atlasImg, x - 7, y +1, RECTARG(character->srcLeftFootBack), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 3,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ - 2,
        }
    });

    TE_Img_blitEx(img, &atlasImg, x - 7, y +4, RECTARG(character->srcLeftFootBack), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 3,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ - 2,
        }
    });

    TE_Img_blitEx(img, &atlasImg, x - 4, y, RECTARG(character->srcRightHand), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 3,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ - 1,
        }
    });


}

void Character_update(Character *character, RuntimeContext *ctx, TE_Img *img, float tx, float ty, int8_t dirX, int8_t dirY)
{
    int baseR = 4;
    // float prevX = character->prevX;
    // float prevY = character->prevY;
    
    character->prevX = character->x;
    character->prevY = character->y;
    float dx = tx - character->x;
    float dy = ty - character->y;
    // TE_Img_line(img, (int16_t) character->x, (int16_t) character->y, (int16_t) tx, (int16_t) ty, DB32Colors[9], (TE_ImgOpState) {
    //             .zCompareMode = Z_COMPARE_LESS,
    //             .zValue = (uint8_t) character->y + 8,
    //         });
    // int8_t signX = dx < 0.0f ? -1 : (dx > 0.0f ? 1 : 0);
    // int8_t signY = dy < 0.0f ? -1 : (dy > 0.0f ? 1 : 0);
    float len = sqrtf(dx * dx + dy * dy);
    character->targetDistance = len;
    if (len < 1.25f)
    {
        dx = 0.0f;
        dy = 0.0f;
        len = 0.0f;
    }
    if (len > 1.5f)
    {
        dx /= len;
        dy /= len;
        len = 1.0f;
    }
    character->lifeTime += ctx->deltaTime;
    float stepSize = ctx->deltaTime * 16.0f;
    float nextX = character->x + dx * stepSize;
    float nextY = character->y + dy * stepSize;
    int16_t tnx, tny;
    int16_t nextBaseX = (nextX + .5f) - 1;
    int16_t nextBaseY = (nextY + .5f) + 6;
    // trying to stabilize movement when colliding by looking if the next step is already
    // colliding with the environment
    float sdf = Environment_calcSDFValue(nextBaseX, nextBaseY, &tnx, &tny);
    if (sdf < baseR)
    {
        character->speed = 0.0f;
        int16_t tndx = tnx - nextBaseX;
        int16_t tndy = tny - nextBaseY;
        float tndlen = sqrtf(tndx * tndx + tndy * tndy);
        if (tndlen > 0.0f)
        {
            float ndx = tndx / tndlen;
            float ndy = tndy / tndlen;
            float push = (baseR - sdf - 0.f);
            // TE_Debug_drawLine(nextX, nextY, tnx, tny, 0xff0000ff);
            nextX = nextX - ndx * push;
            nextY = nextY - ndy * push;
        }
    }
    
    character->x = nextX;
    character->y = nextY;
    if (!character->maskDir)
    {
        character->dirX = dx < -0.25f ? -1 : (dx > 0.25f ? 1 : character->dirX);
        character->dirY = dy < -0.25f ? -1 : (dy > 0.25f ? 1 : character->dirY);
    }

    int16_t x = (int16_t) floorf(character->x + .5f);
    int16_t y = (int16_t) floorf(character->y + .5f - character->flyHeight);
    int16_t charBaseY = (int16_t) floorf(character->y + .5f);
    int baseX = x - 1;
    int baseY = y + 6;
    uint8_t charZ = (uint8_t) character->y + 12;

    int16_t collideCenterX, collideCenterY, collideRadius;
    if (
        Environment_raycastCircle(baseX, baseY, baseR, &collideCenterX, &collideCenterY, &collideRadius) >= 0 ||
        Characters_raycastCircle(character, baseX, baseY, baseR, &collideCenterX, &collideCenterY, &collideRadius) > 0
        )
    {
        int16_t dx = collideCenterX - baseX;
        int16_t dy = collideCenterY - baseY;
        // TE_Img_line(img, baseX, baseY, collideCenterX, collideCenterY, DB32Colors[9], (TE_ImgOpState) {
        //             .zCompareMode = Z_COMPARE_LESS,
        //             .zValue = charZ + 8,
        //         });
        float len = sqrtf(dx * dx + dy * dy);
        if (len > 0.0f && len < baseR + collideRadius)
        {
            float push = (baseR + collideRadius - len);
            // LOG("Push %.2f-%.2f", push, stepSize * 1.5f);
            // if (push <= stepSize * 2.5f)
            // {
            //     character->x = prevX;
            //     character->y = prevY;
            // }
            // else
            // {
            character->x = character->x - dx / len * push;
            character->y = character->y - dy / len * push;
            // }
        }
        // TE_Img_lineCircle(img, baseX, baseY, 4, DB32Colors[9], (TE_ImgOpState) {
        //             .zCompareMode = Z_COMPARE_LESS,
        //             .zValue = charZ + 8,
        //         });
    }

    float actualDx = character->x - character->prevX;
    float actualDy = character->y - character->prevY;
    float actualLen = sqrtf(actualDx * actualDx + actualDy * actualDy);

    character->walkDistance += actualLen;
    if (character->speed != character->speed)
    {
        LOG("Speed NaN fix");
        character->speed = 0.0f;
    }
    character->speed = character->speed * .75f + (actualLen / ctx->deltaTime * 0.25f);
    // TE_Debug_drawText(nextBaseX, nextBaseY, TE_StrFmt("%.0f", character->speed), 0xff0000ff);


    uint8_t walkPhase = 0;
    uint8_t walkPhase1 = 0;
    uint8_t walkPhase2 = 0;
    if (character->speed > 15.0f)
    {
        walkPhase = (int)(character->lifeTime * 10) % 2;
        walkPhase1 = (int)(character->lifeTime * 10 + 2.5f) % 2;
        walkPhase2 = (int)(character->lifeTime * 5 + .5f) % 2;
    }

    // Drawing character
    TE_Rect srcHead = character->dirY < 0 ? character->srcHeadBack : character->srcHeadFront;
    TE_Img_blitEx(img, &atlasImg, x - 8, y - 6, RECTARG(srcHead), (BlitEx) {
        .flipX = character->dirX < 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 8,
        }
    });

    TE_Rect srcBody = character->dirY < 0 ? character->srcBodyBack : character->srcBodyFront;
    TE_Img_blitEx(img, &atlasImg, x - 8, y - 8 + 10 - walkPhase2, RECTARG(srcBody), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 7,
        }
    });

    // feet
    TE_Rect srcLeftFoot = character->dirY < 0 ? character->srcLeftFootBack : character->srcLeftFootFront;
    TE_Img_blitEx(img, &atlasImg, x - 8, y - 8 + 14 - walkPhase, 
        RECTARG(srcLeftFoot), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 6,
        }
    });
    TE_Img_blitEx(img, &atlasImg, x - 2, y - 8 + 14 - (walkPhase1) % 2, 
        RECTARG(srcLeftFoot), (BlitEx) {
        .flipX = 1,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 6,
        }
    });

    // hands
    
    int16_t rHandX = x - 2 + 2 + (dirX < 0 ? 3 : 1) * dirX;
    int16_t rHandY = y - 8 + 11 + dirY;
    int16_t lHandX = x - 2 - 7 + (dirX < 0 ? 1 : 3) * dirX;
    int16_t lHandY = y - 8 + 11 + dirY;
    TE_Img_blitEx(img, &atlasImg, rHandX, rHandY, RECTARG(character->srcRightHand), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 7 + dirY * 2,
        }
    });
    TE_Img_blitEx(img, &atlasImg, lHandX, lHandY, RECTARG(character->srcRightHand), (BlitEx) {
        .flipX = 1,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 7 + dirY * 2,
        }
    });

    // items
    int itemRight = character->itemRightHand;
    int itemLeft = character->itemLeftHand;
    if (itemRight != 0)
    {
        uint8_t id = itemRight < 0 ? -itemRight : itemRight;
        Item *item = &items[id - 1];
        int8_t pivotX = item->pivotX;
        int8_t flipX = itemRight < 0;
        if (dirX < 0)
        {
            flipX = !flipX;
            pivotX = item->src.width - pivotX;
        }
        uint8_t isDrawingAnimation = 0;
        uint8_t selectedAnimationId = item->idleAnimationId;
        
        if (character->isAiming)
        {
            selectedAnimationId = item->aimAnimationId;
        }
        if (character->isStriking)
        {
            selectedAnimationId = item->attackAnimationId;
        }
        else if (character->isHitting)
        {
            selectedAnimationId = item->hitAnimationId;
        }
        if (selectedAnimationId)
        {
            character->runningAnimationTime += ctx->deltaTime;
            uint32_t msTick = (uint32_t)(character->runningAnimationTime * 1000);

            isDrawingAnimation = GameAssets_drawAnimation(selectedAnimationId, img,
                msTick, rHandX - 4, rHandY + 1, 1,
                (BlitEx) {
                    .flipX = flipX,
                    .flipY = 0,
                    .rotate = 0,
                    .tint = 0,
                    .blendMode = TE_BLEND_ALPHAMASK,
                    .tintColor = 0xffffffff,
                    .state = {
                        .zCompareMode = Z_COMPARE_LESS_EQUAL,
                        .zValue = charZ + 6 + dirY * 2,
                    }
                });
            if (!isDrawingAnimation && (character->isStriking || character->isHitting))
            {
                character->isStriking = 0;
                character->isHitting = 0;
                character->runningAnimationTime = 0;

                isDrawingAnimation = GameAssets_drawAnimation(item->idleAnimationId, img,
                    msTick, rHandX - 4, rHandY + 1, 1,
                    (BlitEx) {
                        .flipX = flipX,
                        .flipY = 0,
                        .rotate = 0,
                        .tint = 0,
                        .blendMode = TE_BLEND_ALPHAMASK,
                        .tintColor = 0xffffffff,
                        .state = {
                            .zCompareMode = Z_COMPARE_LESS,
                            .zValue = charZ + 7 + dirY * 2,
                        }
                    });
            }
        }
        if (!isDrawingAnimation)
            TE_Img_blitEx(img, &atlasImg, 
                rHandX + dirX - pivotX, rHandY - item->pivotY, 
                RECTARG(item->src), (BlitEx) {
                .flipX = flipX,
                .flipY = 0,
                .rotate = 0,
                .tint = 0,
                .blendMode = TE_BLEND_ALPHAMASK,
                .tintColor = 0xffffffff,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS,
                    .zValue = charZ + 7 + dirY * 2,
                }
            });
    }

    if (itemLeft != 0)
    {
        uint8_t id = itemLeft < 0 ? -itemLeft : itemLeft;
        Item *item = &items[id - 1];
        int8_t pivotDir = dirX | 1;
        TE_Img_blitEx(img, &atlasImg, 
            lHandX + dirX - item->pivotX * pivotDir, lHandY - item->pivotY, 
            RECTARG(item->src), (BlitEx) {
            .flipX = (itemLeft < 0) ^ (dirX < 0),
            .flipY = 0,
            .rotate = 0,
            .tint = 0,
            .blendMode = TE_BLEND_ALPHAMASK,
            .tintColor = 0xffffffff,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = charZ + 7 + dirY * 2,
            }
        });
    }

    // shadow
    TE_Img_blitSprite(img, GameAssets_getSprite(SPRITE_CHARACTER_SHADOW), x-1, charBaseY + 12, (BlitEx) {
        .blendMode = TE_BLEND_ALPHAMASK,
        .tint = 1,
        .tintColor = 0x44000000,
        
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = 8,
            .zAlphaBlend = 1,
        }
    });
}