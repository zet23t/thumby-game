#include "game_assets.h"
#include "game.h"
#include "fnt_medium.h"

typedef struct SpriteData
{
    uint8_t x, y;
    uint8_t width;
    uint8_t height;
    uint8_t pivotX;
    uint8_t pivotY;
} SpriteData;

static const SpriteData _sprites[] = {
    {0, 249, 8, 7, 4, 3}, // fallback
    {2, 64, 10, 10, 5, 4}, // ROBIN_HEAD_FRONT
    {0, 138, 7, 6, 3,3}, // CART_WHEEL_SIDE
    {0, 127, 16, 10, 8, 4}, // CART_SIDE
    {16, 125, 16, 7, 8, 2}, // CART_GOLD
    {0, 240, 6, 7, 3, 3}, // ARROW_RIGHT
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
    return (TE_Sprite) {
        .img = &atlasImg,
        .src = { .x = data.x, .y = data.y, .width = data.width, .height = data.height },
        .pivotX = data.pivotX,
        .pivotY = data.pivotY,
    };
}

static TE_Img mediumImg;

TE_Font GameAssets_getFont(uint8_t index)
{
    switch (index)
    {
        default:
        {
            mediumImg = (TE_Img) {
                    .p2width = fnt_medium_p2width,
                    .p2height = fnt_medium_p2height,
                    .data = (uint32_t*) fnt_medium_data,
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