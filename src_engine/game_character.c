#include "game.h"
#include <math.h>

#define RECTARG(r) r.x, r.y, r.width, r.height

void Character_update(Character *character, RuntimeContext *ctx, TE_Img *img, float tx, float ty, int8_t dirX, int8_t dirY)
{
    float dx = tx - character->x;
    float dy = ty - character->y;
    int8_t signX = dx < 0.0f ? -1 : (dx > 0.0f ? 1 : 0);
    int8_t signY = dy < 0.0f ? -1 : (dy > 0.0f ? 1 : 0);
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
    character->walkDistance += len;
    character->speed = len / ctx->deltaTime;
    character->x += dx * ctx->deltaTime * 16.0f;
    character->y += dy * ctx->deltaTime * 16.0f;
    character->dirX = dx < -0.25f ? -1 : (dx > 0.25f ? 1 : character->dirX);
    character->dirY = dy < -0.25f ? -1 : (dy > 0.25f ? 1 : character->dirY);

    uint8_t walkPhase = 0;
    uint8_t walkPhase1 = 0;
    uint8_t walkPhase2 = 0;
    if (character->speed > 0.0f)
    {
        walkPhase = (int)(character->lifeTime * 10) % 2;
        walkPhase1 = (int)(character->lifeTime * 10 + 2.5f) % 2;
        walkPhase2 = (int)(character->lifeTime * 5 + .5f) % 2;
    }

    uint8_t charZ = (uint8_t) character->y + 12;
    int16_t x = (int16_t) floorf(character->x);
    int16_t y = (int16_t) floorf(character->y);
    TL_Rect srcHead = character->dirY < 0 ? character->srcHeadBack : character->srcHeadFront;
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

    TL_Rect srcBody = character->dirY < 0 ? character->srcBodyBack : character->srcBodyFront;
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
    TL_Rect srcLeftFoot = character->dirY < 0 ? character->srcLeftFootBack : character->srcLeftFootFront;
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
            .zValue = charZ + 7 + dirY,
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
            .zValue = charZ + 7 + dirY,
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
}