#ifndef __TE_FONT_H__
#define __TE_FONT_H__

#include <inttypes.h>

#include "TE_Image.h"

typedef struct TE_Font
{
    TE_Img *atlas;
    uint16_t glyphCount;
    const uint16_t *glyphValues;
    const uint16_t *rectXs;
    const uint16_t *rectYs;
    const uint8_t *rectWidths;
    const uint8_t *rectHeights;
} TE_Font;

int TE_Font_drawChar(TE_Img *img, TE_Font *font, uint16_t x, uint16_t y, char c, uint32_t color, TE_ImgOpState state);
int TE_Font_drawText(TE_Img *img, TE_Font *font, uint16_t x, uint16_t y, int8_t spacing, const char *text, uint32_t color, TE_ImgOpState state);


#endif // __TE_FONT_H__