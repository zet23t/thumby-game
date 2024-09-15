#include "game_scene_0_testing.h"
#include "game_player.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_environment.h"
#include "game_assets.h"
#include "game_particlesystem.h"
#include "game.h"

#include "TE_sdfmap.h"
#include "TE_math.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int8_t _scene0Mode = 0;

void Scene_0_init()
{
    _scene0Mode = 0;
    Player_setInputEnabled(0);
    Player_setWeapon(0);
    playerCharacter.x = 256;
    playerCharacter.y = 256;
    playerCharacter.targetX = 256;
    playerCharacter.targetY = 256;
    player.x = 256;
    player.y = 256;
    Environment_init();
}

static int _getFibonacciInt(int n)
{
    if (n == 0)
    {
        return 0;
    }
    if (n == 1)
    {
        return 1;
    }
    return _getFibonacciInt(n - 1) + _getFibonacciInt(n - 2);
}

static int _getFirstFibonacciFloat(float n)
{
    if (n == 0)
    {
        return 0.0f;
    }
    if (n == 1)
    {
        return 1.0f;
    }
    return _getFirstFibonacciFloat(n - 1) + _getFirstFibonacciFloat(n - 2);
}

static uint32_t _testInt0(RuntimeContext *ctx, TE_Img *screen)
{
    int n = 20;
    uint32_t start = ctx->getUTime();
    int result = _getFibonacciInt(n);
    uint32_t end = ctx->getUTime();
    TE_Font font = GameAssets_getFont(FONT_TINY);
    char buffer[64];
    snprintf(buffer, 64, "Fibonacci(%d) =\n %d", n, result);
    TE_Font_drawText(screen, &font, 2, 20, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
    return end - start;
}

static uint32_t _testFloat1(RuntimeContext *ctx, TE_Img *screen)
{
    float n = 20.0f;
    uint32_t start = ctx->getUTime();
    float result = _getFirstFibonacciFloat(n);
    uint32_t end = ctx->getUTime();
    TE_Font font = GameAssets_getFont(FONT_TINY);
    char buffer[64];
    snprintf(buffer, 64, "Fibonacci(%.2f) =\n %.2f", n, result);
    TE_Font_drawText(screen, &font, 2, 20, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
    return end - start;
}

static uint32_t _testInt2(RuntimeContext *ctx, TE_Img *screen)
{
    int n = 200000;
    int rndValues[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    int sumdots = 0;
    uint32_t start = ctx->getUTime();
    for (int i = 0; i < n; i += 3)
    {
        int n = i % 17;
        sumdots += rndValues[n] * rndValues[n] + rndValues[n + 1] * rndValues[n + 1] + rndValues[n + 2] * rndValues[n + 2];
        if (sumdots > 1000)
        {
            sumdots -= 1000000;
        }
    }
    uint32_t end = ctx->getUTime();

    TE_Font font = GameAssets_getFont(FONT_TINY);
    char buffer[64];
    snprintf(buffer, 64, "sumdots(%d) =\n %d", n, sumdots);
    TE_Font_drawText(screen, &font, 2, 20, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
    return end - start;
}

static uint32_t _testFloat2(RuntimeContext *ctx, TE_Img *screen)
{
    int n = 200000;
    float rndValues[] = {0.23f, 1.189f, 2.189f, 3.189f, 4.189f, 5.189f, 6.189f, 7.189f, 
        8.189f, 9.189f, 10.189f, 11.189f, 12.189f, 13.189f, 14.189f, 15.189f, 16.189f, 17.189f, 18.189f, 19.189f, 20.189f};
    float sumdots = 0;
    uint32_t start = ctx->getUTime();
    for (int i = 0; i < n; i += 3)
    {
        int n = i % 17;
        sumdots += rndValues[n] * rndValues[n] + rndValues[n + 1] * rndValues[n + 1] + rndValues[n + 2] * rndValues[n + 2];
        if (sumdots > 1000.0f)
        {
            sumdots -= 1000000.0f;
        }
    }
    uint32_t end = ctx->getUTime();

    TE_Font font = GameAssets_getFont(FONT_TINY);
    char buffer[64];
    snprintf(buffer, 64, "sumdots(%d) =\n %.2f", n, sumdots);
    TE_Font_drawText(screen, &font, 2, 20, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
    return end - start;
}

static uint32_t _testFloat3(RuntimeContext *ctx, TE_Img *screen)
{
    int n = 1000;
    uint32_t start = ctx->getUTime();
    float sum = 0.0f;
    for (float f=0.0f;f<n;f+=0.15f)
    {
        sum += sinf(f) + cosf(f * 1.1f);
    }
    uint32_t end = ctx->getUTime();

    TE_Font font = GameAssets_getFont(FONT_TINY);
    char buffer[64];
    snprintf(buffer, 64, "sum(%d) =\n %.2f", n, sum);
    TE_Font_drawText(screen, &font, 2, 20, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
    return end - start;
}

void Scene_0_update(RuntimeContext *ctx, TE_Img *screen)
{
    TE_Img_clear(screen, 0, 0);
    int tests = 4;

    if (ctx->inputLeft && !ctx->prevInputLeft)
    {
        _scene0Mode--;
        if (_scene0Mode < 0)
        {
            _scene0Mode = tests;
        }
        LOG("Scene 0 mode: %d", _scene0Mode);
    }
    if (ctx->inputRight && !ctx->prevInputRight)
    {
        _scene0Mode++;
        if (_scene0Mode > tests)
        {
            _scene0Mode = 0;
        }
    }

    const char *modeName = "None";
    uint32_t elapsed = 0;
    if (_scene0Mode == 0)
    {
        elapsed = _testInt0(ctx, screen);
        modeName = "TestInt0";
    }
    if (_scene0Mode == 1)
    {
        elapsed = _testFloat1(ctx, screen);
        modeName = "TestFloat1";
    }
    if (_scene0Mode == 2)
    {
        elapsed = _testInt2(ctx, screen);
        modeName = "TestInt2";
    }
    if (_scene0Mode == 3)
    {
        elapsed = _testFloat2(ctx, screen);
        modeName = "TestFloat2";
    }
    if (_scene0Mode == 4)
    {
        elapsed = _testFloat3(ctx, screen);
        modeName = "TestFloat3";
    }
    char buffer[64];
    snprintf(buffer, 64, "%s %.2fms", modeName, (float)elapsed / 1000.0f);
    TE_Font font = GameAssets_getFont(FONT_MEDIUM);
    TE_Font_drawText(screen, &font, 2, 2, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
}