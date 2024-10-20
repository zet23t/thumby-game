#include "engine_main.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "game_audio.h"

#ifdef __EMSCRIPTEN__
#define DLL_EXPORT __attribute__((visibility("default")))
#else
    #ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
    #else
    #define DLL_EXPORT
    #endif
#endif

#include <atlas.h>
#include <fnt_myfont.h>
#include <fnt_tiny.h>

#include "TE_Image.h"
#include "TE_rand.h"
#include "TE_Font.h"
#include "game.h"
#include "game_projectile.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_player.h"
#include "game_menu.h"
#include "game_environment.h"
#include "game_scenes.h"
#include "game_assets.h"
#include "game_particlesystem.h"
#include "game_renderobjects.h"
#include "stdarg.h"

DLL_EXPORT void audioUpdate(AudioContext *audioContext);

uint32_t DB32Colors[] = {
    0xFF000000, 0xFF342022, 0xFF3C2845, 0xFF313966, 0xFF3B568F, 0xFF2671DF, 0xFF66A0D9, 0xFF9AC3EE,
    0xFF36F2FB, 0xFF50E599, 0xFF30BE6A, 0xFF6E9437, 0xFF2F694B, 0xFF244B52, 0xFF393C32, 0xFF743F3F,
    0xFF826030, 0xFFE16E5B, 0xFFFF9B63, 0xFFE4CD5F, 0xFFFCDBCB, 0xFFFFFFFF, 0xFFB7AD9B, 0xFF877E84,
    0xFF6A6A69, 0xFF525659, 0xFF8A4276, 0xFF3232AC, 0xFF6357D9, 0xFFBA7BD7, 0xFF4A978F, 0xFF306F8A,
    0xFF36535E, 0xFF48687D, 0xFF3C7EA0, 0xFFC7C3C2, 0xFFE0E0E0,
};

char* TE_StrFmt(const char *format, ...)
{
    static char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    return buffer;
}

static void (*ctxLog)(const char *text);
static void (*ctxSetRGB)(uint8_t r, uint8_t g, uint8_t b);
static void (*ctxPanic)(const char *text);

void TE_DebugRGB(uint8_t r, uint8_t g, uint8_t b)
{
    if (ctxSetRGB)
    {
        ctxSetRGB(r, g, b);
    }
}

void TE_Panic(const char *text)
{
    if (ctxPanic)
    {
        ctxPanic(text);
    }
}

void TE_Logf(const char *tag, const char *fmt, ...)
{
    char buffer[256];
    sprintf(buffer, "[%s] ", tag);
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer + strlen(buffer), fmt, args);
    va_end(args);
    printf("%s\n", buffer);
    if (ctxLog)
    {
        ctxLog(buffer);
    }
}

const char* formatFileRef(const char *file, int line)
{
    static char buffer[256];
    sprintf(buffer, "%s:%d", file, line);
    return buffer;
}

typedef struct Avg32F
{
    float values[32];
    uint8_t index;
} Avg32F;

void Avg32F_fill(Avg32F *avg, float value)
{
    for (int i=0;i<32;i++)
    {
        avg->values[i] = value;
    }
}

void Avg32F_push(Avg32F *avg, float value)
{
    avg->values[avg->index] = value;
    avg->index = (avg->index + 1) % 32;
}

float Avg32F_get(Avg32F *avg)
{
    float sum = 0;
    for (int i=0;i<32;i++)
    {
        sum += avg->values[i];
    }
    return sum / 32.0f;
}

static TE_Img img;

void TE_Debug_drawPixel(int x, int y, uint32_t color)
{
    TE_Img_setPixel(&img, x, y, color, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}

void TE_Debug_drawLine(int x1, int y1, int x2, int y2, uint32_t color)
{
    TE_Img_line(&img, x1, y1, x2, y2, color, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}

void TE_Debug_drawLineCircle(int x, int y, int r, uint32_t color)
{
    TE_Img_lineCircle(&img, x, y, r, color, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}


TE_Img tinyImg;
TE_Font tinyfont;

void TE_Debug_drawText(int x, int y, const char *text, uint32_t color)
{
    TE_Font_drawText(&img, &tinyfont, x, y, -1, text, color, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}
uint8_t activeSceneId;

//# WebAssembly function helpers
// functions for web assembly export to ease JS bridging
#include <stdlib.h>
DLL_EXPORT AudioContext* AudioContext_create()
{
    AudioContext *ctx = (AudioContext*)malloc(sizeof(AudioContext));
    memset(ctx, 0, sizeof(AudioContext));

    return ctx;
}

DLL_EXPORT void* AudioContext_getChannelStatus(AudioContext *audioCtx, int *size)
{
    *size = sizeof(SFXChannelStatus) * SFX_CHANNELS_COUNT;
    return audioCtx->outSfxChannelStatus;
}

DLL_EXPORT void AudioContext_setSfxInstructions(AudioContext *audioCtx, void *data)
{
    memcpy(audioCtx->inSfxInstructions, data, sizeof(SFXInstruction) * SFX_CHANNELS_COUNT);
}

DLL_EXPORT void* RuntimeContext_getSfxInstructions(RuntimeContext *ctx, int *size)
{
    *size = sizeof(SFXInstruction) * SFX_CHANNELS_COUNT;
    return ctx->outSfxInstructions;
}

DLL_EXPORT void RuntimeContext_setSfxChannelStatus(RuntimeContext *ctx, void *data)
{
    memcpy(ctx->sfxChannelStatus, data, sizeof(SFXChannelStatus) * SFX_CHANNELS_COUNT);
}

DLL_EXPORT void RuntimeContext_clearSfxInstructions(RuntimeContext *ctx)
{
    memset(ctx->outSfxInstructions, 0, sizeof(SFXInstruction) * SFX_CHANNELS_COUNT);
}

DLL_EXPORT void AudioContext_audioUpdate(AudioContext *audioCtx, int sampleRate, int sampleSize, void *buffer, int frameCount)
{
    audioCtx->frames = frameCount;
    audioCtx->sampleRate = sampleRate;
    audioCtx->sampleSize = sampleSize;
    audioCtx->outBuffer = (char*)buffer;
    audioUpdate(audioCtx);
}



DLL_EXPORT RuntimeContext* RuntimeContext_create()
{
    RuntimeContext *ctx = (RuntimeContext*)malloc(sizeof(RuntimeContext));
    memset(ctx, 0, sizeof(RuntimeContext));
    return ctx;
}

DLL_EXPORT uint32_t* RuntimeContext_getScreen(RuntimeContext *ctx) { return ctx->screenData; }
DLL_EXPORT uint32_t RuntimeContext_getRGBLed(RuntimeContext *ctx) { return ctx->rgbLightRed | (ctx->rgbLightGreen << 8) | (ctx->rgbLightBlue << 16); }
DLL_EXPORT float RuntimeContext_getRumble(RuntimeContext *ctx) { return ctx->rumbleIntensity; }
DLL_EXPORT void RuntimeContext_setUTimeCallback(RuntimeContext *ctx, uint32_t (*getUTime)())
{
    ctx->getUTime = getUTime;
}
DLL_EXPORT void RuntimeContext_updateInputs(RuntimeContext *ctx, 
    double time, double timeDelta,
    uint8_t up, uint8_t right, uint8_t down, uint8_t left, uint8_t a, uint8_t b, uint8_t menu, uint8_t shoulderLeft, uint8_t shoulderRight)
{
    ctx->time = (float)time;
    ctx->deltaTime = (float)timeDelta;
    ctx->frameCount++;

    ctx->prevInputUp = ctx->inputUp;
    ctx->prevInputRight = ctx->inputRight;
    ctx->prevInputDown = ctx->inputDown;
    ctx->prevInputLeft = ctx->inputLeft;
    ctx->prevInputA = ctx->inputA;
    ctx->prevInputB = ctx->inputB;
    ctx->prevInputMenu = ctx->inputMenu;
    ctx->prevInputShoulderLeft = ctx->inputShoulderLeft;
    ctx->prevInputShoulderRight = ctx->inputShoulderRight;

    ctx->inputUp = up;
    ctx->inputRight = right;
    ctx->inputDown = down;
    ctx->inputLeft = left;
    ctx->inputA = a;
    ctx->inputB = b;
    ctx->inputMenu = menu;
    ctx->inputShoulderLeft = shoulderLeft;
    ctx->inputShoulderRight = shoulderRight;
}

//# Runtime function hooks

//## init
DLL_EXPORT void init(RuntimeContext *ctx)
{
    ctxLog = ctx->log;
    ctxSetRGB = ctx->dbgSetRGB;
    ctxPanic = ctx->panic;
    TE_DebugRGB(0, 0, 0);
    TE_Logf(LOG_TAG_SYSTEM, "Initializing, sizeof(RuntimeContext) = %d", sizeof(RuntimeContext));
    TE_Logf(LOG_TAG_SYSTEM, "sizeof(TE_Img) = %d; sizeof(TE_ImgOpState) = %d", sizeof(TE_Img), sizeof(TE_ImgOpState));
    TE_Logf(LOG_TAG_SYSTEM, "sizeof(RenderObjectSprite) = %d;", sizeof(RenderObjectSprite));
    TE_Logf(LOG_TAG_SYSTEM, "sizeof(TE_Font) = %d;", sizeof(TE_Font));
    
    TE_DebugRGB(1, 0, 0);
    ParticleSystem_init();

    atlasImg = (TE_Img) {
        .p2width = atlas_p2width,
        .p2height = atlas_p2height,
        .data = (uint32_t*) atlas_data,
    };

    items[0] = (Item) {
        .pivotX = 2,
        .pivotY = 4,
        .src = { .x = 0, .y = 96, .width = 6, .height = 13 },
    };
    items[1] = (Item) {
        .pivotX = 6,
        .pivotY = 4,
        .src = { .x = 7, .y = 96, .width = 9, .height = 13 },
    };
    
    items[ITEM_PIKE - 1] = (Item) {
        .pivotX = 1,
        .pivotY = 8,
        .src = { .x = 64, .y = 80, .width = 8, .height = 16 },
    };

    items[ITEM_STAFF - 1] = (Item) {
        .pivotX = 0,
        .pivotY = 8,
        .src = { .x = 16, .y = 96, .width = 3, .height = 16 },
        .idleAnimationId = ANIMATION_STAFF_IDLE,
        .aimAnimationId = ANIMATION_STAFF_AIM,
        .attackAnimationId = ANIMATION_STAFF_ATTACK,
        .hitAnimationId = ANIMATION_STAFF_ATTACK_HIT,
        .cooldown = 0.25f,
        .meleeRange = 7,
    };

    playerCharacter = (Character)
    {
        .x = player.x,
        .y = player.y,
        .itemRightHand = -1,
        .srcHeadFront = { .x = 0, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 0, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40, .y = 64, .width = 8, .height = 6 },
    };

    // Henchman
    characters[1] = (Character)
    {
        .srcHeadFront = { .x = 48, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48, .y = 64, .width = 8, .height = 6 },
    };
    // Dinglewort
    characters[2] = (Character)
    {
        .srcHeadFront = { .x = 48 * 2, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48 * 2, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48 * 2, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48 * 2, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48 * 2, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48 * 2, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48 * 2, .y = 64, .width = 8, .height = 6 },
    };
    // bandit 1
    characters[3] = (Character)
    {
        .srcHeadFront = { .x = 48 * 3, .y = 64, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 48 * 3, .y = 64 + 16, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48 * 3, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48 * 3, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48 * 3, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48 * 3, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48 * 3, .y = 64, .width = 8, .height = 6 },
    };
    // bandit 2
    characters[4] = (Character)
    {
        .srcHeadFront = { .x = 160, .y = 80, .width = 15, .height = 11 },
        .srcHeadBack = { .x = 176, .y = 80, .width = 15, .height = 11 },
        .srcBodyFront = { .x = 16+48 * 3, .y = 64, .width = 15, .height = 6 },
        .srcBodyBack = { .x = 16+48 * 3, .y = 64 + 8, .width = 15, .height = 6 },
        .srcLeftFootFront = { .x = 32+48 * 3, .y = 64, .width = 8, .height = 6 },
        .srcLeftFootBack = { .x = 32+48 * 3, .y = 72, .width = 8, .height = 6 },
        .srcRightHand = { .x = 40+48 * 3, .y = 64, .width = 8, .height = 6 },
    };

    TE_Logf(LOG_TAG_SYSTEM, "Initialized");
    TE_Logf(LOG_TAG_SYSTEM, "sizeof(items) = %d; sizeof(characters) = %d", 
        sizeof(items), sizeof(characters));
    TE_DebugRGB(0, 1, 0);

    GameRuntimeContextState *state = (GameRuntimeContextState*)ctx->projectData;
    Scene_init(ctx, state->isInitiailized ? state->currentScene : 1);
    TE_DebugRGB(1, 1, 1);
    Scene_setStep(state->currentStep);
    TE_DebugRGB(1, 0, 1);
}

//## audioUpdate
DLL_EXPORT void audioUpdate(AudioContext *audioContext)
{
    GameAudio_update(audioContext);

    // audioFrequency = frequency + (audioFrequency - frequency)*0.95f;

    // float incr = audioFrequency/(float)sampleRate;
    // short *d = (short *)buffer;
    // float amplitude = 0.1f;

    // for (unsigned int i = 0; i < frames; i++)
    // {
    //     d[i] = (short)(32000.0f*sinf(2*M_PI*sineIdx) * amplitude);
    //     sineIdx += incr;
    //     if (sineIdx > 1.0f) sineIdx -= 1.0f;
    // }
}

//## update
// The main update function for the game engine
DLL_EXPORT void update(RuntimeContext *ctx)
{
    TE_FrameStats imgStats = TE_Img_resetStats();

    ctx->frameStats = (TE_FrameStats) {0};
    if (ctx->inputShoulderLeft && !ctx->prevInputShoulderLeft)
    {
        Scene_init(ctx, (Scene_getCurrentSceneId() - 1)%8);
    }
    if (ctx->inputShoulderRight && !ctx->prevInputShoulderRight)
    {
        Scene_init(ctx, (Scene_getCurrentSceneId() + 1)%8);
    }
    img = (TE_Img) {
        .p2width = 7,
        .p2height = 7,
        .data = ctx->screenData,
    };
    
    tinyImg = (TE_Img) {
        .p2width = fnt_tiny_p2width,
        .p2height = fnt_tiny_p2height,
        .data = (uint32_t*) fnt_tiny_data,
    };

    tinyfont = (TE_Font) {
        .atlas = &tinyImg,
        .glyphCount = fnt_tiny_glyph_count,
        .glyphValues = fnt_tiny_glyphs_values,
        .rectXs = fnt_tiny_glyphs_rects_x,
        .rectYs = fnt_tiny_glyphs_rects_y,
        .rectWidths = fnt_tiny_glyphs_rects_width,
        .rectHeights = fnt_tiny_glyphs_rects_height,
    };

    
    TE_Img_clear(&img, DB32Colors[13], 0);


    TE_randSetSeed(Scene_getCurrentSceneId() * 127 + 392);
    for (int i=0;i<24;i++)
    {
        int x = TE_randRange(0, 120);
        int y = TE_randRange(0, 120);
        TE_Img_blitEx(&img, &atlasImg, x, y, 16, 0, 8, 8, (BlitEx) {
            .flipX = TE_randRange(0, 2),
            .flipY = 0,
            .rotate = 0,
            .tint = 1,
            .blendMode = TE_BLEND_ALPHAMASK,
            .tintColor = 0xff306f8a,
        });
    }
    uint32_t start = ctx->getUTime();
    BENCH(Scene_update(ctx, &img), scene)
    BENCH(Projectiles_update(projectiles, ctx, &img), projectiles)
    BENCH(RenderObject_update(ctx, &img), renderObjects)
    TE_randSetSeed(ctx->frameCount * 1 + 392);
    BENCH(Enemies_update(ctx, &img), enemies)
    BENCH(Player_update(&player, &playerCharacter, ctx, &img), player)
    
    BENCH(Environment_update(ctx, &img), environment)
    BENCH(ParticleSystem_update(ctx, &img), particles)
    BENCH(ScriptedAction_update(ctx, &img), script)
    BENCH(Menu_update(ctx, &img), menu)
    uint32_t end = ctx->getUTime();
    ctx->frameStats.updateTime.total = end - start;

    // GameAssets_drawAnimation(ANIMATION_STAFF_HIT, &img, ctx->time * 1000.0f, 64, 64, 10, (BlitEx) {
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .tintColor = 0xffffffff,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_ALWAYS,
    //         .zValue = 100,
    //     }
    // });


    // logo rot test
    // TE_Img_blitEx(&img, &atlasImg, 16,16,8,232,72,24, (BlitEx){
    //     .flipX = 0,
    //     .flipY = 0,
    //     .rotate = ctx->frameCount / 30 % 4,
    //     .tint = 0,
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .tintColor = 0xffffffff,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_ALWAYS,
    //         .zValue = 255,
    //     }
    // });

    // if (50 > Scene_getStep())
    // {
    //     Scene_setStep(Scene_getStep() + 1);
    // }
    // TE_Logf(LOG_TAG_SYSTEM, "Scene step: %d", Scene_getMaxStep());

    // TE_Font_drawText(&img, &myfont, 2, 2, -1, "Sherwood Forest", 0xffffffff, (TE_ImgOpState) {
    //     .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //     .zValue = 100,
    // });

    if (ctx->drawStats)
    {

        uint16_t fps = (uint16_t)roundl(1.0f / ctx->deltaTime);
        static uint16_t recentFPS[32];
        static uint8_t recentFPSIndex = 0;
        recentFPS[recentFPSIndex] = fps;
        recentFPSIndex = (recentFPSIndex + 1) % 32;

        uint16_t avgFPS = 0;
        for (int i=0;i<32;i++)
        {
            avgFPS += recentFPS[i];
        }
        avgFPS /= 32;
        // if (fps > 25)
        // {
        //     return;
        // }
        char text[64];
        const char *benchNames[] = {
            "scene",
            "projectiles",
            "environment",
            "particles",
            "enemies",
            "player",
            "script",
            "menu",
            "renderO",
            "total",
        };
        static uint8_t displayBenchIndex = 9;
        static Avg32F avgDuration;
        int dir = 0;
        if (ctx->inputB && !ctx->prevInputRight && ctx->inputRight)
        {
            dir = 1;
        }
        if (ctx->inputB && !ctx->prevInputLeft && ctx->inputLeft)
        {
            dir = -1;
        }

        if (dir)
        {
            displayBenchIndex = (displayBenchIndex + 11 + dir) % 11;
            if (displayBenchIndex < 10)
                Avg32F_fill(&avgDuration, ctx->frameStats.updateTimes[displayBenchIndex] / 1000.0f);
        }

        if (displayBenchIndex == 10)
        {
            sprintf(text, "FPS: %d|%d|%dk|%d", avgFPS, imgStats.blitCount, imgStats.blitPixelCount>>10, imgStats.blitXCount);
        }
        else
        {
            float durationMS = ctx->frameStats.updateTimes[displayBenchIndex] / 1000.0f;
            Avg32F_push(&avgDuration, durationMS);
            float durationAvg = Avg32F_get(&avgDuration);
            sprintf(text, "FPS: %d; %s %.2f", avgFPS, benchNames[displayBenchIndex], durationAvg);
        }

        TE_Font medFont = GameAssets_getFont(FONT_MEDIUM);
        TE_Font_drawText(&img, &medFont, 1, 115, -1, text, 0xffffffff, (TE_ImgOpState) {
            .zCompareMode = Z_COMPARE_ALWAYS,
            .zValue = 255,
        });
    }

    GameRuntimeContextState *state = (GameRuntimeContextState*)ctx->projectData;
    state->isInitiailized = 1;
    state->currentScene = Scene_getCurrentSceneId();
    state->currentStep = Scene_getStep();

    #ifdef PLATFORM_DESKTOP
    ctx->frameStats = TE_Img_getStats();
    uint32_t maxOverdraw = 0;
    for (int i=0;i<128*128;i++)
    {
        maxOverdraw = ctx->frameStats.overdrawCount[i] > maxOverdraw ? ctx->frameStats.overdrawCount[i] : maxOverdraw;
    }
    #endif
}