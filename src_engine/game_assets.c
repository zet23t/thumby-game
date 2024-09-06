#include "game_assets.h"
#include "game.h"
#include "fnt_medium.h"
#include "fnt_myfont.h"
#include "game_particlesystem.h"
#include "TE_rand.h"

typedef struct SpriteData
{
    uint8_t x, y;
    uint8_t width;
    uint8_t height;
    uint8_t pivotX;
    uint8_t pivotY;
} SpriteData;

static const SpriteData _sprites[] = {
    {0, 249, 8, 7, 4, 3},    // fallback
    {2, 64, 10, 10, 5, 4},   // ROBIN_HEAD_FRONT
    {0, 138, 7, 6, 3, 3},    // CART_WHEEL_SIDE
    {0, 127, 16, 10, 8, 4},  // CART_SIDE
    {16, 125, 16, 7, 8, 2},  // CART_GOLD
    {0, 240, 6, 7, 3, 3},    // ARROW_RIGHT
    {161, 48, 4, 7, 2, 4},   // POLE_TOP
    {32, 128, 20, 12, 3, 0}, // ANIM_HAHAHA_R_F1
    {52, 128, 15, 10, 1, 0}, // ANIM_HAHAHA_R_F2
    {68, 128, 9, 7, 0, 0},   // ANIM_HAHAHA_R_F3
    {78, 128, 6, 4, 0, 0},   // ANIM_HAHAHA_R_F4
    {80, 112, 18, 11, 3, 0}, // ANIM_HAHAHA_L_F1
    {82, 123, 13, 13, 8, 0}, // ANIM_HAHAHA_L_F2
    {98, 112, 13, 9, 7, 0},  // ANIM_HAHAHA_L_F3
    {16, 96, 3, 16, 1, 8},   // ANIM_STAFF_HIT_F1
    {20, 97, 11, 13, 5, 6},  // ANIM_STAFF_HIT_F2
    {32, 96, 16, 3, 8, 1},   // ANIM_STAFF_HIT_F3
};

static SpriteData _getSpriteData(uint8_t index)
{
    if (index >= sizeof(_sprites) / sizeof(SpriteData))
        return _sprites[0];
    return _sprites[index];
}

TE_Sprite GameAssets_getSprite(uint8_t index)
{
    SpriteData data = _getSpriteData(index);
    return (TE_Sprite){
        .img = &atlasImg,
        .src = {.x = data.x, .y = data.y, .width = data.width, .height = data.height},
        .pivotX = data.pivotX,
        .pivotY = data.pivotY,
    };
}

#define HAHA_FRAME_DURATION 500
static int DrawAnimation_HAHAHA(TE_Img *dst, uint32_t msTick, uint8_t spriteId, int16_t x, int16_t y, BlitEx blitEx, int loopCount)
{
    // for (int n=0; n<2; n++)
    // {
    msTick *= 2;
    if (msTick >= HAHA_FRAME_DURATION * 5 * loopCount - 2)
        return 0;

    int ftime = msTick % (HAHA_FRAME_DURATION * 5);
    if (ftime > HAHA_FRAME_DURATION * 2)
    {
        uint8_t alpha = 255 - (ftime - HAHA_FRAME_DURATION * 2) * 255 / (HAHA_FRAME_DURATION * 3);
        blitEx.tint = 1;
        blitEx.tintColor = (blitEx.tintColor & 0xffffff) | (alpha << 24);
        blitEx.state.zAlphaBlend = 1;
    }
    int yoff = (-ftime * 2 / HAHA_FRAME_DURATION * 3 - ftime * ftime / (HAHA_FRAME_DURATION * HAHA_FRAME_DURATION * 20)) / 3 + y;
    int fselect = ftime % (HAHA_FRAME_DURATION);
    int animOffset = ftime > HAHA_FRAME_DURATION ? 1 : 0;
    if (fselect < HAHA_FRAME_DURATION / 2)
    {
        TE_Img_blitSprite(dst, GameAssets_getSprite(spriteId + animOffset), x, yoff, blitEx);
    }
    else
    {
        TE_Img_blitSprite(dst, GameAssets_getSprite(spriteId + 1 + animOffset), x + 2, yoff + 3, blitEx);
    }
    return 1;
}

#define STAFF_HIT_FRAME_DURATION 60
static int DrawAnimation_STAFF_HIT(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, int isHit, BlitEx blitEx)
{
    uint32_t frame = msTick / STAFF_HIT_FRAME_DURATION;
    if (frame >= 6 * maxLoopCount)
        return 0;
    frame %= 6;
    // TE_Debug_drawPixel(x, y, 0xffff00ff);
    if (frame == 2 && isHit)
    {
        // no clue if should do something here
    }
    if (frame == 4 || frame == 3) frame = 2;
    if (frame < 3)
        TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F1 + frame), x, y, blitEx);
    else
    {
        blitEx.flipX = 1;
        blitEx.flipY = 1;
        TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F2), x, y, blitEx);
    }

    return 1;
}

static int DrawAnimation_STAFF_IDLE(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F1), x, y, blitEx);
    return 1;
}


int GameAssets_drawAnimation(uint8_t index, TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    switch (index)
    {
    case ANIMATION_HAHAHA_RIGHT:
        return DrawAnimation_HAHAHA(dst, msTick, SPRITE_ANIM_HAHAHA_R_F1, x, y, blitEx, maxLoopCount);
    case ANIMATION_HAHAHA_LEFT:
        return DrawAnimation_HAHAHA(dst, msTick, SPRITE_ANIM_HAHAHA_L_F1, x, y, blitEx, maxLoopCount);
        break;
    case ANIMATION_STAFF_ATTACK_HIT:
        return DrawAnimation_STAFF_HIT(dst, msTick, x, y, maxLoopCount, 1, blitEx);
    case ANIMATION_STAFF_ATTACK:
        return DrawAnimation_STAFF_HIT(dst, msTick, x, y, maxLoopCount, 0, blitEx);
    case ANIMATION_STAFF_IDLE:
        return DrawAnimation_STAFF_IDLE(dst, msTick, x, y, maxLoopCount, blitEx);
    }
    return 0;
}

static TE_Img mediumImg;
static TE_Img myfontImg;

TE_Font GameAssets_getFont(uint8_t index)
{
    switch (index)
    {
    case FONT_LARGE:
    {
        myfontImg = (TE_Img){
            .p2width = fnt_myfont_p2width,
            .p2height = fnt_myfont_p2height,
            .data = (uint32_t *)fnt_myfont_data,
        };
        return (TE_Font){
            .atlas = &myfontImg,
            .glyphCount = fnt_myfont_glyph_count,
            .glyphValues = fnt_myfont_glyphs_values,
            .rectXs = fnt_myfont_glyphs_rects_x,
            .rectYs = fnt_myfont_glyphs_rects_y,
            .rectWidths = fnt_myfont_glyphs_rects_width,
            .rectHeights = fnt_myfont_glyphs_rects_height,
        };
    }
    default:
    {
        mediumImg = (TE_Img){
            .p2width = fnt_medium_p2width,
            .p2height = fnt_medium_p2height,
            .data = (uint32_t *)fnt_medium_data,
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

        return mediumfont;
    }
    }
}