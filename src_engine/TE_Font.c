#include "TE_Font.h"
#include "game_assets.h"
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <string.h>

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

// appends a string to another string that may contain special font instructions. Does not check for buffer overflow.
char* TE_Font_concat(char *dest, const char *src)
{
    char *eostring = &dest[TE_Font_getStringLength(dest)];
    int srcLength = TE_Font_getStringLength(src);
    memcpy(eostring, src, srcLength);
    eostring[srcLength] = 0;
    return dest;
}

// determines the count of bytes of a string that may contain special font instructions
int TE_Font_getStringLength(const char *text)
{
    const char *start = text;
    while (*text)
    {
        if (*text == '\b')
        {
            text++;
            switch (*text)
            {
                case 'x': // move x
                {
                    text+=2;
                    continue;
                }
                case 's': // sprite injection
                {
                    text+=4;
                    continue;
                }
            }
        }
        text++;
    }
    return text - start;
}

int TE_Font_drawText(TE_Img *img, TE_Font *font, int16_t x, int16_t y, int8_t spacing, const char *text, uint32_t color, TE_ImgOpState state)
{
    int width = 0;
    while (*text)
    {
        if (*text == '\r') continue;
        if (*text == '\n')
        {
            y += font->rectHeights[0];
            text++;
            width = 0;
            continue;
        }
        if (*text == '\b')
        {
            text++;
            switch (*text)
            {
                case 'x': // move x
                {
                    width += (int8_t)text[1];
                    text+=2;
                    continue;
                }
                case 's': // sprite injection
                {
                    uint8_t spriteId = (uint8_t)text[1];
                    int8_t offsetX = text[2];
                    int8_t offsetY = text[3];
                    text+=4;
                    TE_Sprite sprite = GameAssets_getSprite(spriteId);
                    TE_Img_blitSprite(img, sprite, x + width + offsetX + sprite.pivotX, y + offsetY + sprite.pivotY, (BlitEx){
                        .blendMode = TE_BLEND_ALPHAMASK,
                        .state = state,
                    });
                    width += sprite.src.width + spacing;
                    continue;
                }
            }
        }
        width += TE_Font_drawChar(img, font, x + width, y, *text, color, state) + spacing;
        text++;
    }
    return width;
}

typedef struct TE_Font_drawTextBoy_args
{
    TE_Img *img;
    TE_Font *font;
    int16_t x;
    int16_t y;
    uint8_t w;
    uint8_t h;
    int8_t wordSpacing;
    int8_t lineSpacing;
    const char *text;
    float alignX;
    float alignY;
    uint32_t color;
    TE_ImgOpState state;
    int draw;
} TE_Font_drawTextBoy_args;

static TE_Vector2_s16 TE_Font_drawTextBox_internal(TE_Font_drawTextBoy_args args)
{
    TE_Img *img = args.img;
    TE_Font *font = args.font;
    int16_t x = args.x;
    int16_t y = args.y;
    uint8_t w = args.w;
    int8_t wordSpacing = args.wordSpacing;
    int8_t lineSpacing = args.lineSpacing;
    const char *text = args.text;
    float alignX = args.alignX;
    uint32_t color = args.color;
    TE_ImgOpState state = args.state;
    int draw = args.draw;

    int textIndex = 0;
    char line[256];
    int16_t lineY = y;
    int16_t maxWidth = 0;
    // if (!draw) TE_Img_lineRect(img, x, y, w, h, 0xff0000ff, state);
    while (text[textIndex])
    {
        memset(line, 0, sizeof(line));
        strncpy(line, &text[textIndex], sizeof(line) - 1);
        uint8_t charWidth;
        int lineWidth = TE_Font_getLetterWidth(font, &text[textIndex], &charWidth);
        // printf("?%d ",lineWidth);
        int lineEndIndex = textIndex;
        int linePos = 0;
        for (int i=0;i<charWidth;i++) line[linePos++] = text[textIndex + i];
        int breakPos = lineEndIndex + charWidth;
        int breakWidth = lineWidth;
        while (text[lineEndIndex + charWidth] && lineWidth < w && text[lineEndIndex + charWidth] != '\n')
        {
            int cw = TE_Font_getLetterWidth(font, &text[lineEndIndex + charWidth], &charWidth);

            if (lineWidth + cw < w)
            {
                lineEndIndex += charWidth;
                lineWidth += cw + wordSpacing;
            }
            else
            {
                line[breakPos - textIndex] = 0;
                lineEndIndex = breakPos;
                lineWidth = breakWidth;
                break;
            }

            if (text[lineEndIndex + charWidth] <= ' ')
            {
                breakWidth = lineWidth;
                breakPos = lineEndIndex + (text[lineEndIndex + charWidth] ? charWidth + 1 : charWidth);
            }
        }

        if (breakWidth > maxWidth)
        {
            maxWidth = breakWidth;
        }
        textIndex = breakPos;
        if (text[textIndex] == '\n') textIndex += 1;

        // line[linePos + charWidth - 1] = 0;
        // printf("line: %d,%d %s %d %d\n",lineX,lineY,line, lineWidth, w);
        if (draw) {
            int16_t lineX = x + (int16_t)ceilf((w - lineWidth) * alignX);
            // LOG("line: %s %d %d", line, charWidth, lineWidth);
            TE_Font_drawText(img, font, lineX, lineY, wordSpacing, line, color, state);
        }
        // TE_Img_lineRect(img, lineX, lineY, lineWidth, font->rectHeights[0], 0xff00ffff, (TE_ImgOpState){.zValue=255});
        lineY += font->rectHeights[0] + lineSpacing;
    }
    return (TE_Vector2_s16){
        .x = maxWidth,
        .y = lineY - y - lineSpacing
    };
}

TE_Vector2_s16 TE_Font_drawTextBox(TE_Img *img, TE_Font *font, int16_t x, int16_t y, uint8_t w, uint8_t h, int8_t wordSpacing, int8_t lineSpacing, const char *text, float alignX, float alignY, uint32_t color, TE_ImgOpState state)
{
    TE_Font_drawTextBoy_args args = (TE_Font_drawTextBoy_args) {
        .img = img,
        .font = font,
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .wordSpacing = wordSpacing,
        .lineSpacing = lineSpacing,
        .text = text,
        .alignX = alignX,
        .alignY = alignY,
        .color = color,
        .state = state,
        .draw = 0
    };
    TE_Vector2_s16 size = TE_Font_drawTextBox_internal(args);
    int16_t offset = (int16_t)ceilf((h - size.y) * alignY);
    args.y += offset;
    args.draw = 1;
    TE_Font_drawTextBox_internal(args);
    return size;
}

int TE_Font_getLetterWidth(TE_Font *font, const char *c, uint8_t *charWidth)
{
    if (*c == '\b')
    {
        switch (c[1])
        {
            case 'x': // move x
            {
                *charWidth = 2;
                return (int8_t)c[2];
            }
            case 's': // sprite injection
            {
                *charWidth = 4;
                uint8_t spriteId = (uint8_t)c[2];
                TE_Sprite sprite = GameAssets_getSprite(spriteId);
                return sprite.src.width;
            }
        }
        return 0;
    }

    *charWidth = 1;
    for (uint16_t i = 0; i < font->glyphCount; i++)
    {
        if (font->glyphValues[i] == *c)
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