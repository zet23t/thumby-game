#ifndef __TE_IMAGE_H__
#define __TE_IMAGE_H__

#include <inttypes.h>

typedef struct TE_Img
{
    uint8_t p2width;
    uint8_t p2height;
    uint32_t *data;
} TE_Img;

#define Z_COMPARE_ALWAYS 0
#define Z_COMPARE_EQUAL 1
#define Z_COMPARE_LESS 2
#define Z_COMPARE_GREATER 3
#define Z_COMPARE_LESS_EQUAL 4
#define Z_COMPARE_GREATER_EQUAL 5
#define Z_COMPARE_NOT_EQUAL 6

typedef struct TE_ImgOpState
{
    uint8_t zCompareMode : 3;
    uint8_t zNoWrite : 1;
    uint8_t zValue;
} TE_ImgOpState;


#define TE_BLEND_NONE 0
#define TE_BLEND_ALPHAMASK 1

typedef struct BlitEx
{
    uint8_t flipX : 1;
    uint8_t flipY : 1;
    uint8_t rotate : 2;
    uint8_t tint : 1;
    uint8_t blendMode : 3;

    uint32_t tintColor;
    TE_ImgOpState state;
} BlitEx;

void TE_Img_setPixel(TE_Img *img, uint16_t x, uint16_t y, uint32_t color, TE_ImgOpState state);
void TE_Img_fillTriangle(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color, TE_ImgOpState state);
uint32_t TE_Img_getPixelEx(TE_Img *img, uint16_t ox, uint16_t oy, uint16_t x, uint16_t y, uint16_t w, uint16_t h, BlitEx options);
void TE_Img_line(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, TE_ImgOpState state);
uint32_t TE_Color_tint(uint32_t color, uint32_t tint);
void TE_Img_blitEx(TE_Img *img, TE_Img *src, int16_t x, int16_t y, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height, BlitEx options);
void TE_Img_clear(TE_Img *img, uint32_t color, uint8_t z);

#endif