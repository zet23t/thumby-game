#include "TE_Font.h"
#include <stdio.h>
#include <math.h>

int TE_Font_drawChar(TE_Img *img, TE_Font *font, int16_t x, int16_t y, char c, uint32_t color, TE_ImgOpState state)
{
    for (uint16_t i = 0; i < font->glyphCount; i++)
    {
        if (font->glyphValues[i] == c)
        {
            TE_Img_blitEx(img, font->atlas, x, y, font->rectXs[i], font->rectYs[i], font->rectWidths[i], font->rectHeights[i], (BlitEx){
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

int TE_Font_drawText(TE_Img *img, TE_Font *font, int16_t x, int16_t y, int8_t spacing, const char *text, uint32_t color, TE_ImgOpState state)
{
    int width = 0;
    while (*text)
    {
        width += TE_Font_drawChar(img, font, x + width, y, *text, color, state) + spacing;
        text++;
    }
    return width;
}

static int TE_Font_drawTextBox_internal(TE_Img *img, TE_Font *font, int16_t x, int16_t y, uint8_t w, uint8_t h, int8_t wordSpacing, int8_t lineSpacing, const char *text, float alignX, float alignY, uint32_t color, TE_ImgOpState state, int draw)
{
    int textIndex = 0;
    char line[256];
    int16_t lineY = y;
    if (!draw) TE_Img_lineRect(img, x, y, w, h, 0xff0000ff, state);
    while (text[textIndex])
    {
        int lineWidth = TE_Font_getLetterWidth(font, text[textIndex]);
        // printf("?%d ",lineWidth);
        int lineEndIndex = textIndex;
        line[0] = text[textIndex];
        int linePos = 1;
        int breakPos = lineEndIndex + 1;
        int breakWidth = lineWidth;
        while (text[lineEndIndex + 1] && lineWidth < w && text[lineEndIndex + 1] != '\n')
        {
            int cw = TE_Font_getLetterWidth(font, text[lineEndIndex + 1]);
            if (lineWidth + cw < w)
            {
                line[linePos++] = text[lineEndIndex + 1];
                lineWidth += cw + wordSpacing;
                lineEndIndex += 1;
            }
            else
            {
                line[breakPos - textIndex] = 0;
                lineEndIndex = breakPos;
                lineWidth = breakWidth;
                break;
            }

            if (text[lineEndIndex + 1] <= ' ')
            {
                breakWidth = lineWidth;
                breakPos = lineEndIndex + (text[lineEndIndex + 1] ? 2 : 1);
            }
        }

        textIndex = breakPos;
        if (text[textIndex] == '\n') textIndex += 1;

        line[linePos] = 0;
        // printf("line: %d,%d %s %d %d\n",lineX,lineY,line, lineWidth, w);
        if (draw) {
            int16_t lineX = x + (int16_t)ceilf((w - lineWidth) * alignX);
            TE_Font_drawText(img, font, lineX, lineY, wordSpacing, line, color, state);
        }
        // TE_Img_lineRect(img, lineX, lineY, lineWidth, font->rectHeights[0], 0xff00ffff, (TE_ImgOpState){.zValue=255});
        lineY += font->rectHeights[0] + lineSpacing;
    }
    return lineY - y - lineSpacing;
}

int TE_Font_drawTextBox(TE_Img *img, TE_Font *font, int16_t x, int16_t y, uint8_t w, uint8_t h, int8_t wordSpacing, int8_t lineSpacing, const char *text, float alignX, float alignY, uint32_t color, TE_ImgOpState state)
{
    int height = TE_Font_drawTextBox_internal(img, font, x, y, w, h, wordSpacing, lineSpacing, text, alignX, alignY, color, state, 0);
    int16_t offset = (int16_t)ceilf((h - height) * alignY);
    TE_Font_drawTextBox_internal(img, font, x, y + offset, w, h, wordSpacing, lineSpacing, text, alignX, alignY, color, state, 1);
    return height;
}

int TE_Font_getLetterWidth(TE_Font *font, char c)
{
    for (uint16_t i = 0; i < font->glyphCount; i++)
    {
        if (font->glyphValues[i] == c)
        {
            return font->rectWidths[i];
        }
    }

    // default to first letter (probably space)
    return font->rectWidths[0];
}

int TE_Font_getWidth(TE_Font *font, const char *text, int8_t spacing)
{
    int width = 0;
    while (*text)
    {
        for (uint16_t i = 0; i < font->glyphCount; i++)
        {
            if (font->glyphValues[i] == *text)
            {
                width += font->rectWidths[i] + spacing;
                break;
            }
        }
        text++;
    }
    return width - spacing;
}