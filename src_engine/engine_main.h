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


#define DB32_BLACK 0
#define DB32_DEEPINDIGO 1
#define DB32_DEEPPURPLE 2
#define DB32_DARKBROWN 3
#define DB32_BROWN 4
#define DB32_ORANGE 5
#define DB32_DARKTAN 6
#define DB32_TAN 7
#define DB32_YELLOW 8
#define DB32_LIGHTGREEN 9
#define DB32_FELTGREEN 12
#define DB32_DARKGREEN 14
#define DB32_DARKBLUE 15
#define DB32_BLUE 17
#define DB32_SKYBLUE 18
#define DB32_CYAN 19
#define DB32_LIGHTBLUE 20
#define DB32_WHITE 21
#define DB32_LIGHTGRAY 23
#define DB32_GRAY 24
#define DB32_DARKGRAY 25
#define DB32_PURPLE 26
#define DB32_RED 27
#define DB32_PINK 29
#define DB32_FIELDGREEN 30

#define LOG_TAG_SYSTEM "SYSTEM"
void TE_Logf(const char *tag, const char *format, ...);

#endif // __ENGINE_MAIN_H__