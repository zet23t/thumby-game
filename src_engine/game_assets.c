#include "game_assets.h"
#include "game.h"
#include "fnt_medium.h"
#include "fnt_myfont.h"

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
    {68, 128, 9, 6, 0, 0},   // ANIM_HAHAHA_R_F3
    {78, 128, 6, 4, 0, 0},   // ANIM_HAHAHA_R_F4
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

static void DrawAnimation_HAHAHA_RIGHT(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, BlitEx blitEx, int loopCount)
{
#define HAHA_FRAME_DURATION 500
    // for (int n=0; n<2; n++)
    // {
    msTick *= 2;
    if (msTick > HAHA_FRAME_DURATION * 5 * loopCount)
        return;

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
        TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_HAHAHA_R_F1 + animOffset), x, yoff, blitEx);
    }
    else
    {
        TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_HAHAHA_R_F2 + animOffset), x + 2, yoff + 3, blitEx);
    }
    // else if (fselect < HAHA_FRAME_DURATION * 3)
    // {
    //     TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_HAHAHA_R_F3), x + 16, y - 5, blitEx);
    // }
    // else if (fselect < HAHA_FRAME_DURATION * 4)
    // {
    //     TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_HAHAHA_R_F4), x + 20, y - 7, blitEx);
    // }
    //     if (msTick < HAHA_FRAME_DURATION * 3)
    //         break;
    //     msTick -= HAHA_FRAME_DURATION;
    // }
}

void GameAssets_drawAnimation(uint8_t index, TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    switch (index)
    {
    case ANIMATION_HAHAHA_RIGHT:
        DrawAnimation_HAHAHA_RIGHT(dst, msTick, x, y, blitEx, maxLoopCount);
        break;

    default:
        break;
    }
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