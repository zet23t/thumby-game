#ifndef __ENGINE_MAIN_H__
#define __ENGINE_MAIN_H__

#include <inttypes.h>

typedef struct RuntimeContext
{
    uint32_t screenData[128*128];

    union {
        uint32_t inputState;
        struct
        {
            uint32_t inputUp:1;
            uint32_t inputDown:1;
            uint32_t inputLeft:1;
            uint32_t inputRight:1;
            uint32_t inputA:1;
            uint32_t inputB:1;
            uint32_t inputShoulderLeft:1;
            uint32_t inputShoulderRight:1;
            uint32_t inputMenu:1;
        };
    };
    union {
        uint32_t previousInputState;
        struct
        {
            uint32_t prevInputUp:1;
            uint32_t prevInputDown:1;
            uint32_t prevInputLeft:1;
            uint32_t prevInputRight:1;
            uint32_t prevInputA:1;
            uint32_t prevInputB:1;
            uint32_t prevInputShoulderLeft:1;
            uint32_t prevInputShoulderRight:1;
            uint32_t prevInputMenu:1;
        };
    };

    uint32_t frameCount;
    float time;
    float deltaTime;
} RuntimeContext;

extern uint32_t DB32Colors[];

#define LOG_TAG_SYSTEM "SYSTEM"
void TE_Logf(const char *tag, const char *format, ...);

#endif // __ENGINE_MAIN_H__