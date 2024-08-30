#include "game_scenes.h"
#include "game_enemies.h"
#include "game_player.h"
#include "game_assets.h"
#include "game_environment.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>

typedef struct Scene
{
    uint8_t id;
    void (*initFn)();
    void (*updateFn)(RuntimeContext *ctx, TE_Img *screenData);
} Scene;

static void (*_sceneUpdateFn)(RuntimeContext *ctx, TE_Img *screenData);

static void DrawTextBlock(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, const char *text)
{
    TE_Font font = GameAssets_getFont(0);
    TE_Img_fillRect(screenData, x, y, width, height, DB32Colors[21], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Img_lineRect(screenData, x, y, width, height, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Font_drawTextBox(screenData, &font, x + 4, y + 4, width - 8, height - 8, -1, -4, text, 0.5f, 0.5f, 0xffffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}

static void DrawSpeechBubble(TE_Img *screenData, int16_t x, int16_t y, int16_t width, int16_t height, int16_t arrowX, int16_t arrowY, const char *text)
{
    TE_Font font = GameAssets_getFont(0);
    // TE_Img_drawPatch9(screenData, &font.atlas, x, y, width, height, 0, 0, 0, 0, (BlitEx){
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //         .zValue = 0,
    //     }
    // });
    int baseW = 8;
    int cx = arrowX > x + baseW * 2 ? (arrowX < x + width - baseW * 2 ? arrowX : x + width - baseW * 2) : x + baseW * 2;
    int cy = y + height / 2;
    TE_Img_fillTriangle(screenData, arrowX, arrowY, cx - baseW, cy, cx + baseW, cy, DB32Colors[21], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    TE_Img_lineTriangle(screenData, arrowX, arrowY, cx - baseW, cy, cx + baseW, cy, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    

    TE_Img_lineRect(screenData, x, y, width, height, DB32Colors[1], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS,
        .zValue = 255,
    });
    TE_Img_fillRect(screenData, x+1, y+1, width-2, height-2, DB32Colors[21], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
    // printf("Drawing text: %s %p\n", text, font.atlas->data);
    TE_Font_drawTextBox(screenData, &font, x + 4, y + 4, width - 8, height - 8, -1, -4, text, 0.5f, 0.5f, 0xffffffff, (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_ALWAYS,
        .zValue = 255,
    });
}

static void Cart_draw(TE_Img *screenData, int16_t x, int16_t y, uint8_t loaded, RuntimeContext *ctx)
{
    TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_CART_SIDE), x, y, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = y + 8,
        }
    });
    switch(loaded)
    {
        case 1:
            TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_CART_GOLD), x, y-3, (BlitEx){
                .blendMode = TE_BLEND_ALPHAMASK,
                .state = {
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = y + 8,
                }
            });
            TE_randSetSeed(ctx->frameCount / 4);
            int sparks = TE_randRange(1, 3);
            for (int i=0;i<sparks;i++)
            {
                int sparkX = TE_randRange(x-7, x+1);
                int sparkY = TE_randRange(y-5, y+1);
                TE_Img_setPixel(screenData, sparkX, sparkY, DB32Colors[21], (TE_ImgOpState){
                    .zCompareMode = Z_COMPARE_LESS_EQUAL,
                    .zValue = y+8,
                });
            }
        break;

    }
    TE_Img_blitSprite(screenData, GameAssets_getSprite(SPRITE_CART_WHEEL_SIDE), x-1, y+5, (BlitEx){
        .blendMode = TE_BLEND_ALPHAMASK,
        .state = {
            .zCompareMode = Z_COMPARE_LESS_EQUAL,
            .zValue = y + 8,
        }
    });
    
    TE_Img_fillRect(screenData, x-7, y+4, 16, 4, DB32Colors[14], (TE_ImgOpState){
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 0,
    });
}

static void Scene_1_Update(RuntimeContext *ctx, TE_Img *screenData)
{
    // Update scene 1
    int16_t cartX = player.x - 20;
    
    const uint16_t pathQHeight = 16;
    for (uint16_t x=0;x<128;x+=2)
    {
        uint16_t srcX = x % 16;
        int16_t y = 50 + (int16_t)(sin((x) * 0.05f) * 5.0f + x * 0.1f);

        TE_Img_blitEx(screenData, &atlasImg, x, y, 71 + srcX, 16, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
        TE_Img_blitEx(screenData, &atlasImg, x, y+pathQHeight, 71 + srcX, 16 + 24, 2, pathQHeight, (BlitEx){
            .blendMode = TE_BLEND_ALPHAMASK,
            .state = {
                .zCompareMode = Z_COMPARE_LESS_EQUAL,
                .zValue = 0,
            }
        });
    }
    // TE_Img_drawPatch9(screenData, &atlasImg, -8, 58, 140, 28, 64,16, 12, 12, (BlitEx){
    //     .blendMode = TE_BLEND_ALPHAMASK,
    //     .state = {
    //         .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //         .zValue = 0,
    //     }
    // });

    int16_t cartY = 60 + (int16_t)(sin((cartX) * 0.05f) * 5.0f + cartX * 0.1f);
    int16_t cartYAnchor = 60 + (int16_t)(sin((cartX+6) * 0.05f) * 5.0f + (cartX + 6) * 0.1f);



    player.y = 54 + (int16_t)(sin((player.x) * 0.05f) * 5.0f + player.x * 0.1f);
    player.aimY = 1;
    player.dirY = 1;
    player.dy = 1;
    playerCharacter.y = player.y;
    playerCharacter.dirY = 1;

    Cart_draw(screenData, cartX, cartY,1, ctx);

    TE_Img_line(screenData, cartX+8, cartYAnchor, cartX+20, player.y + 6, DB32Colors[3], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });
    TE_Img_line(screenData, cartX+8, cartYAnchor + 1, cartX+20, player.y + 7, DB32Colors[2], (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 83,
    });

    const char *text = NULL;

    if (player.x > 108)
    {
        text = NULL;
    }
    else if (player.x > 80)
    {
        text = "I can now get the pool I always wanted!";
    }
    else if (player.x > 60)
    {
        text = "Finally I can retire.";
    }
    else if (player.x > 40)
    {
        text = "So. Much. Loot!";
    }
    else if (player.x > 20)
    {
        text = "The crusades were awesome.";
    }

    if (text)
    {
        DrawSpeechBubble(screenData, 10,20, 108, 30, cartX + 20, player.y - 5, text);
    }

    if (!ctx->inputRight)
    {
        DrawTextBlock(screenData, 70, 105, 48, 16, "Press >");
    }

    if (player.x < -8)
    {
        player.x = -8;
        playerCharacter.x = player.x;
    }
    if (cartX > 130)
    {
        Scene_init(2);
    }
}

static void Scene_1_init()
{
    // Enemies_spawn(1, 28, 42);
    // Enemies_spawn(1, 44, 28);

    player.x = -8;
    playerCharacter.x = player.x;

    Environment_addTreeGroup(24, 30, 1232, 5, 25);
    Environment_addTreeGroup(114, 30, 122, 5, 25);
    Environment_addTreeGroup(114, 125, 1252, 5, 25);
    Environment_addTreeGroup(24, 124, 99, 5, 20);
    // Environment_addTreeGroup(64, 84, 199, 3, 20);
}

static Scene scenes[] = {
    { .id = 1, .initFn = Scene_1_init, .updateFn = Scene_1_Update },
    {0}
};

static void NoSceneUpdate(RuntimeContext *ctx, TE_Img *screenData)
{
    // Do nothing
}

void Scene_init(uint8_t sceneId)
{
    Player_setWeapon(0);
    Environment_init();

    for (int i=0;scenes[i].initFn;i++)
    {
        if (scenes[i].id == sceneId)
        {
            scenes[i].initFn();
            _sceneUpdateFn = scenes[i].updateFn;
            return;
        }
    }

    _sceneUpdateFn = NoSceneUpdate;
}

void Scene_update(RuntimeContext *ctx, TE_Img *screen)
{
    _sceneUpdateFn(ctx, screen);
}