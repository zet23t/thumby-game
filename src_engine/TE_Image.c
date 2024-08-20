#include "TE_Image.h"

void TE_Img_setPixel(TE_Img *img, uint16_t x, uint16_t y, uint32_t color, TE_ImgOpState state)
{
    if (x >= (1 << img->p2width) || y >= (1 << img->p2height))
    {
        return;
    }
    uint32_t *pixel = &img->data[(y << img->p2width) + x];
    uint8_t zDst = *pixel >> 24;
    if ((state.zCompareMode == Z_COMPARE_ALWAYS) ||
        (state.zCompareMode == Z_COMPARE_EQUAL && zDst == state.zValue) ||
        (state.zCompareMode == Z_COMPARE_LESS && zDst < state.zValue) ||
        (state.zCompareMode == Z_COMPARE_GREATER && zDst > state.zValue) ||
        (state.zCompareMode == Z_COMPARE_LESS_EQUAL && zDst <= state.zValue) ||
        (state.zCompareMode == Z_COMPARE_GREATER_EQUAL && zDst >= state.zValue) ||
        (state.zCompareMode == Z_COMPARE_NOT_EQUAL && zDst != state.zValue)
    )
    {
        if (state.zNoWrite)
        {
            color = (color & 0xffffff) | (zDst << 24);
        }
        else
        {
            color = (color & 0xffffff) | (state.zValue << 24);
        }
        *pixel = color;
    }
}


uint32_t TE_Img_getPixelEx(TE_Img *img, uint16_t ox, uint16_t oy, uint16_t x, uint16_t y, uint16_t w, uint16_t h, BlitEx options)
{
    if (options.flipX)
    {
        x = w - x - 1;
    }
    if (options.flipY)
    {
        y = h - y - 1;
    }
    if (options.rotate == 1)
    {
        uint16_t tmp = x;
        x = y;
        y = w - tmp - 1;
    }
    else if (options.rotate == 2)
    {
        x = w - x - 1;
        y = h - y - 1;
    }
    else if (options.rotate == 3)
    {
        uint16_t tmp = x;
        x = h - y - 1;
        y = tmp;
    }

    x += ox;
    y += oy;

    return img->data[(y << img->p2width) + x];
}


static int absi(int x)
{
    return x < 0 ? -x : x;
}

void TE_Img_fillTriangle(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color, TE_ImgOpState state)
{
    if (y0 > y1)
    {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y1 > y2)
    {
        int16_t tmp = y1;
        y1 = y2;
        y2 = tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y0 > y1)
    {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    
    int16_t dx01 = x1 - x0;
    int16_t dy01 = y1 - y0;
    int16_t dx02 = x2 - x0;
    int16_t dy02 = y2 - y0;
    int16_t dx12 = x2 - x1;
    int16_t dy12 = y2 - y1;

    int16_t x, y;
    if (dy01 == 0)
    {
        dy01 = 1;
    }
    if (dy02 == 0)
    {
        dy02 = 1;
    }
    if (dy12 == 0)
    {
        dy12 = 1;
    }
    for (y = y0; y <= y1; y++)
    {
        int16_t xL = x0 + dx01 * (y - y0) / dy01;
        int16_t xR = x0 + dx02 * (y - y0) / dy02;
        if (xL > xR)
        {
            int16_t tmp = xL;
            xL = xR;
            xR = tmp;
        }
        for (x = xL; x <= xR; x++)
        {
            TE_Img_setPixel(img, x, y, color, state);
        }
    }
    for (y = y1; y <= y2; y++)
    {
        int16_t xL = x1 + dx12 * (y - y1) / dy12;
        int16_t xR = x0 + dx02 * (y - y0) / dy02;
        if (xL > xR)
        {
            int16_t tmp = xL;
            xL = xR;
            xR = tmp;
        }
        for (x = xL; x <= xR; x++)
        {
            TE_Img_setPixel(img, x, y, color, state);
        }
    }
}


void TE_Img_line(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, TE_ImgOpState state)
{
    int16_t dx = absi(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = -absi(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (1)
    {
        TE_Img_setPixel(img, x0, y0, color, state);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        int16_t e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

uint32_t TE_Color_tint(uint32_t color, uint32_t tint)
{
    uint32_t r = ((color & 0xFF) * (tint & 0xFF)) >> 8;
    uint32_t g = (((color >> 8) & 0xFF) * ((tint >> 8) & 0xFF)) >> 8;
    uint32_t b = (((color >> 16) & 0xFF) * ((tint >> 16) & 0xFF)) >> 8;
    uint32_t a = (((color >> 24) & 0xFF) * ((tint >> 24) & 0xFF)) >> 8;
    return r | (g << 8) | (b << 16) | (a << 24);
}

void TE_Img_blitEx(TE_Img *img, TE_Img *src, int16_t x, int16_t y, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height, BlitEx options)
{
    for (uint16_t i = 0; i < width; i++)
    {
        for (uint16_t j = 0; j < height; j++)
        {
            uint32_t color = TE_Img_getPixelEx(src, srcX, srcY, i, j, width, height, options);
            if (options.tint)
            {
                color = TE_Color_tint(color, options.tintColor);
            }

            if (options.blendMode == TE_BLEND_ALPHAMASK)
            {
                if ((color & 0xFF000000) == 0)
                {
                    continue;
                }
            }

            TE_Img_setPixel(img, x + i, y + j, color, options.state);
        }
    }
}

void TE_Img_clear(TE_Img *img, uint32_t color, uint8_t z)
{
    color = (color & 0xffffff) | (z << 24);
    for (uint32_t i = 0; i < (1 << img->p2width) * (1 << img->p2height); i++)
    {
        img->data[i] = color;
    }
}