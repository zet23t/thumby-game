#ifndef __ENGINE_MAIN_H__
#define __ENGINE_MAIN_H__

#include <inttypes.h>

#define INPUT_BUTTON_UP 1
#define INPUT_BUTTON_DOWN 2
#define INPUT_BUTTON_LEFT 4
#define INPUT_BUTTON_RIGHT 8
#define INPUT_BUTTON_A 16
#define INPUT_BUTTON_B 32
#define INPUT_BUTTON_SHOULDER_LEFT 64
#define INPUT_BUTTON_SHOULDER_RIGHT 128
#define INPUT_BUTTON_MENU 256


#define ALIGN_VALUE4(x) (((x)+3)&~3)

typedef struct TE_FrameStats
{
    uint32_t blitCount;
    uint32_t blitXCount;
    uint32_t blitPixelCount;
#ifdef PLATFORM_DESKTOP
    uint32_t overdrawCount[128*128];
#endif
    union {
        uint32_t updateTimes[10];
        struct {
            uint32_t scene;
            uint32_t projectiles;
            uint32_t environment;
            uint32_t particles;
            uint32_t enemies;
            uint32_t player;
            uint32_t script;
            uint32_t menu;
            uint32_t renderObjects;
            uint32_t total;
        } updateTime;
    };
} TE_FrameStats;

#define BENCH(call, name) ctx->frameStats.updateTime.name = ctx->getUTime(); call; ctx->frameStats.updateTime.name = ctx->getUTime() - ctx->frameStats.updateTime.name;

typedef struct RuntimeContext
{
    uint32_t screenData[128*128];
    TE_FrameStats frameStats;
    union {
        uint8_t flags;
        struct {
            uint8_t rgbLightRed:1;
            uint8_t rgbLightGreen:1;
            uint8_t rgbLightBlue:1;
        };
    };
    float rumbleIntensity;
    uint8_t projectData[16];

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

    uint8_t isCharging, drawStats;

    uint32_t frameCount;
    float time;
    float deltaTime;

    uint32_t(*getUTime)(void);
    void(*log)(const char *text);
    void(*dbgSetRGB)(uint8_t r, uint8_t g, uint8_t b);
    void(*panic)(const char *text);
} RuntimeContext;

extern uint32_t DB32Colors[];
void TE_DebugRGB(uint8_t r, uint8_t g, uint8_t b);
void TE_Panic(const char *text);

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
const char* formatFileRef(const char *file, int line);
char* TE_StrFmt(const char *format, ...);
#define LOG(...) TE_Logf(formatFileRef(__FILE__,__LINE__), __VA_ARGS__)

#endif // __ENGINE_MAIN_H__