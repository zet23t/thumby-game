#include "TE_Image.h"

#ifndef VARIANT_NAME
#define VARIANT_NAME _TE_Img_blit_base
#endif

static void VARIANT_NAME(TE_Img *img, TE_Img *src, int16_t x, int16_t y, uint16_t srcX, uint16_t srcY, int16_t width, int16_t height, BlitEx options)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    uint16_t imgWidth = 1 << img->p2width;
    uint16_t imgHeight = 1 << img->p2height;
    uint16_t srcWidth = 1 << src->p2width;
    uint16_t srcHeight = 1 << src->p2height;

    TE_ImgOpState state = options.state;

#ifdef VARIANT_TINT
    uint32_t tint = options.tintColor;
    uint8_t tintR = (tint & 0xFF);
    uint8_t tintG = (tint >> 8) & 0xFF;
    uint8_t tintB = (tint >> 16) & 0xFF;
    uint8_t tintA = (tint >> 24) & 0xFF;
#endif

    uint32_t *srcData = src->data;
    uint32_t srcP2width = src->p2width;
    uint32_t *dstData = img->data;
    uint32_t dstP2width = img->p2width;
    uint8_t zValue = state.zValue;

    for (uint16_t j = 0, dstY = y, v = srcY; j < height; j++, dstY++, v++)
    {
        uint32_t srcIndex = (v << srcP2width) + srcX;
        uint32_t dstIndex = (dstY << dstP2width) + x;
        for (uint16_t i = 0, dstX = x; i < width; i++, dstX++, srcIndex++, dstIndex++)
        {
            uint32_t color = srcData[srcIndex];

#ifdef VARIANT_TINT
            {
                uint32_t r = ((color & 0xFF) * tintR) >> 8;
                uint32_t g = (((color >> 8) & 0xFF) * tintG) >> 8;
                uint32_t b = (((color >> 16) & 0xFF) * tintB) >> 8;
                uint32_t a = (((color >> 24) & 0xFF) * tintA) >> 8;
                color = r | (g << 8) | (b << 16) | (a << 24);
            }
#endif

#ifdef VARIANT_ALPHAMASK
            if ((color & 0xFF000000) == 0)
            {
                continue;
            }
#endif
#if PLATFORM_DESKTOP
            frameStats.overdrawCount[(dstX) + (dstY) * 128]++;
#endif
            // TE_Img_setPixelUnchecked(img, dstX, dstY, color, options.state);
            {
                uint32_t *pixel = &dstData[dstIndex];
                uint8_t zDst = *pixel >> 24;
// #ifdef VARIANT_Z_COMPARE_ALWAYS
#ifdef VARIANT_Z_COMPARE_EQUAL
                if (zDst == zValue) 
#endif
#ifdef VARIANT_Z_COMPARE_LESS
                if (zDst < zValue)
#endif
#ifdef VARIANT_Z_COMPARE_GREATER
                if (zDst > zValue)
#endif
#ifdef VARIANT_Z_COMPARE_LESS_EQUAL
                if (zDst <= zValue)
#endif
#ifdef VARIANT_Z_COMPARE_GREATER_EQUAL
                if (zDst >= zValue)
#endif
#ifdef VARIANT_Z_COMPARE_NOT_EQUAL
                if (zDst != zValue)
#endif
                {
#ifdef VARIANT_ALPHA_BLEND
                    if (color & 0xFF000000 < 0xfe000000)
                    {
                        uint32_t a = color >> 24;
                        uint32_t r = (color & 0xFF) * a >> 8;
                        uint32_t g = ((color >> 8) & 0xFF) * a >> 8;
                        uint32_t b = ((color >> 16) & 0xFF) * a >> 8;
                        uint32_t colorDst = *pixel;
                        uint32_t aDst = colorDst & 0xff000000;
                        uint32_t rDst = (colorDst & 0xFF) * (255 - a) >> 8;
                        uint32_t gDst = ((colorDst >> 8) & 0xFF) * (255 - a) >> 8;
                        uint32_t bDst = ((colorDst >> 16) & 0xFF) * (255 - a) >> 8;
                        color = r + rDst | ((g + gDst) << 8) | ((b + bDst) << 16) | aDst;
                    }
#endif

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
        }
    }
}

#undef VARIANT_NAME
#undef VARIANT_TINT
#undef VARIANT_ALPHAMASK
#undef VARIANT_Z_COMPARE_ALWAYS
#undef VARIANT_Z_COMPARE_EQUAL
#undef VARIANT_Z_COMPARE_LESS
#undef VARIANT_Z_COMPARE_GREATER
#undef VARIANT_Z_COMPARE_LESS_EQUAL
#undef VARIANT_Z_COMPARE_GREATER_EQUAL
#undef VARIANT_Z_COMPARE_NOT_EQUAL
