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

int TE_Font_drawChar(TE_Img *img, TE_Font *font, int16_t x, int16_t y, char c, uint32_t color, TE_ImgOpState state);
int TE_Font_drawText(TE_Img *img, TE_Font *font, int16_t x, int16_t y, int8_t spacing, const char *text, uint32_t color, TE_ImgOpState state);
int TE_Font_getWidth(TE_Font *font, const char *text, int8_t spacing);
int TE_Font_getLetterWidth(TE_Font *font, char c);
int TE_Font_drawTextBox(TE_Img *img, TE_Font *font, int16_t x, int16_t y, uint8_t w, uint8_t h, int8_t spacing, const char *text, float alignX, float alignY, uint32_t color, TE_ImgOpState state);
#endif // __TE_FONT_H__