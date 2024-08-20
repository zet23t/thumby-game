#ifndef __ENGINE_MAIN_H__
#define __ENGINE_MAIN_H__

#include <inttypes.h>

typedef struct RuntimeContext
{
    uint32_t screenData[128*128];
    uint32_t inputUp:1;
    uint32_t inputDown:1;
    uint32_t inputLeft:1;
    uint32_t inputRight:1;
    uint32_t inputA:1;
    uint32_t inputB:1;
    uint32_t inputShoulderLeft:1;
    uint32_t inputShoulderRight:1;
    uint32_t inputMenu:1;
    uint32_t frameCount;
    float time;
    float deltaTime;
} RuntimeContext;

typedef struct TL_Rect
{
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} TL_Rect;

extern uint32_t DB32Colors[];

#endif // __ENGINE_MAIN_H__