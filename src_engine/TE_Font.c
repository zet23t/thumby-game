#include "TE_Font.h"

int TE_Font_drawChar(TE_Img *img, TE_Font *font, uint16_t x, uint16_t y, char c, uint32_t color, TE_ImgOpState state)
{
    for (uint16_t i = 0; i < font->glyphCount; i++)
    {
        if (font->glyphValues[i] == c)
        {
            TE_Img_blitEx(img, font->atlas, x, y, font->rectXs[i], font->rectYs[i], font->rectWidths[i], font->rectHeights[i], (BlitEx) {
                .flipX = 0,
                .flipY = 0,
                .rotate = 0,
                .tint = 1,
                .blendMode = TE_BLEND_ALPHAMASK,
                .tintColor = color,
                .state = state,
            });
            return font->rectWidths[i];
        }
    }
    return 0;
}

int TE_Font_drawText(TE_Img *img, TE_Font *font, uint16_t x, uint16_t y, int8_t spacing, const char *text, uint32_t color, TE_ImgOpState state)
{
    int width = 0;
    while (*text)
    {
        width += TE_Font_drawChar(img, font, x + width, y, *text, color, state) + spacing;
        text++;
    }
    return width;
}