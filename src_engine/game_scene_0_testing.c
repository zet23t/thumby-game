#include "game_scene_0_testing.h"
#include "game_player.h"
#include "game_character.h"
#include "game_enemies.h"
#include "game_environment.h"
#include "game_assets.h"
#include "game_particlesystem.h"
#include "game_renderobjects.h"
#include "game.h"

#include "TE_sdfmap.h"
#include "TE_math.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int8_t _scene0Mode = 0;
static int8_t _scene0ModeSelect = 0;

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

    RenderObject_init(0x2000);
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

static uint32_t _testRands(RuntimeContext *ctx, TE_Img *screen)
{
    static int counter = 0;
    counter++;
    int n = 100000;
    TE_randSetSeed(1);
    uint32_t sum = 0;
    uint32_t start = ctx->getUTime();
    for (int i = 0; i < n; i++)
    {
        sum += TE_randRange(-1234, 1234);
    }
    uint32_t end = ctx->getUTime();
    TE_Font font = GameAssets_getFont(FONT_TINY);
    char buffer[64];
    snprintf(buffer, 64, "sum(%d) =\n %d", n, sum);
    TE_Font_drawText(screen, &font, 2, 20, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});

    for (int y= 48; y < 96;y++)
    for (int x=0;x<(counter/8)%128;x++)
    {
        TE_Img_setPixel(screen, x, y, 0xFF|TE_rand(), (TE_ImgOpState){0});
    }
    return end - start;
}

void Scene_0_drawinit(RuntimeContext *ctx, TE_Img *screen)
{
    RenderObject_clear();
    RenderPrefab *prefab = RenderPrefab_create((RenderObjectCounts){
        .spriteMaxCount=1, 
        .atlasBlitMaxCount=1, 
        .atlasBlitSkewXMaxCount=1, 
        .atlasBlitSkewYMaxCount=1, 
        .prefabInstanceMaxCount=32
    });
    // RenderPrefab_addSprite(prefab, (RenderObjectSprite){
    //     .spriteIndex = SPRITE_ANIM_STAFF_HIT_F1,
    //     .x = 32,
    //     .y = 32,
    //     .blitEx = (BlitEx){.flipX=0, .flipY=0, .rotate=0, .state=(TE_ImgOpState){.zValue=0}}
    // });

    for (int row=0; row < 6; row++)
    {
        int8_t y = row * 32 + 16;
        RenderPrefab_addPrefabInstance(prefab, (RenderObjectPrefabInstance){
            .prefab = GameAssets_getRenderPrefab(RENDER_PREFAB_TREE, TE_randRange(1,8)),
            .offsetX = 16,
            .offsetY = y,
            .offsetZ = 0
        });
        RenderPrefab_addPrefabInstance(prefab, (RenderObjectPrefabInstance){
            .prefab = GameAssets_getRenderPrefab(RENDER_PREFAB_TREE, TE_randRange(1,8)),
            .offsetX = 48,
            .offsetY = y,
            .offsetZ = 0
        });
        RenderPrefab_addPrefabInstance(prefab, (RenderObjectPrefabInstance){
            .prefab = GameAssets_getRenderPrefab(RENDER_PREFAB_TREE, TE_randRange(1,8)),
            .offsetX = 72,
            .offsetY = y,
            .offsetZ = 0
        });
        RenderPrefab_addPrefabInstance(prefab, (RenderObjectPrefabInstance){
            .prefab = GameAssets_getRenderPrefab(RENDER_PREFAB_TREE, TE_randRange(1,8)),
            .offsetX = 96,
            .offsetY = y,
            .offsetZ = 0
        });
    }

    RenderObject_setMain(prefab);
}
void Scene_0_draw(RuntimeContext *ctx, TE_Img *screen)
{
    // TE_Debug_drawLineCircle(16, 32, 8, 0xFF00FF00);
    // TE_Debug_drawLineCircle(48, 32, 8, 0xFF00FF00);
    // TE_Debug_drawLineCircle(72, 32, 8, 0xFF00FF00);
}

void Scene_0_bench(RuntimeContext *ctx, TE_Img *screen)
{
    RenderObject_clear();

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
    if (_scene0Mode == 5)
    {
        elapsed = _testRands(ctx, screen);
        modeName = "TestRands";
    }
    char buffer[64];
    snprintf(buffer, 64, "%s %.2fms", modeName, (float)elapsed / 1000.0f);
    TE_Font font = GameAssets_getFont(FONT_MEDIUM);
    TE_Font_drawText(screen, &font, 2, 2, 1, buffer, 0xFFFFFFFF, (TE_ImgOpState){0});
}

void Scene_0_update(RuntimeContext *ctx, TE_Img *screen)
{
    if (_scene0ModeSelect == 0) TE_Img_clear(screen, 0, 0);
    int tests = 5;
    GameRuntimeContextState *state = (GameRuntimeContextState*)ctx->projectData;
    int8_t nextSelect = state->currentConfigA;

    if (ctx->inputUp && !ctx->prevInputUp)
    {
        nextSelect--;
        if (nextSelect < 0)
        {
            nextSelect = 1;
        }
    }
    if (ctx->inputDown && !ctx->prevInputDown)
    {
        nextSelect++;
        if (nextSelect > 1)
        {
            nextSelect = 0;
        }
    }
    if (nextSelect != _scene0ModeSelect)
    {
        _scene0ModeSelect = nextSelect;
        if (_scene0ModeSelect == 1)
        {
            Scene_0_drawinit(ctx, screen);
        }
        LOG("Scene 0 mode select: %d", _scene0ModeSelect);
    }

    state->currentConfigA = _scene0ModeSelect;
    _scene0Mode = state->currentConfigB;

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

    state->currentConfigB = _scene0Mode;

    if (_scene0ModeSelect == 0)
    {
        Scene_0_bench(ctx, screen);
    }
    if (_scene0ModeSelect == 1)
    {
        Scene_0_draw(ctx, screen); 
    }
}