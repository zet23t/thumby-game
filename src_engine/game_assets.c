#include "game_assets.h"
#include "game.h"
#include "fnt_tiny.h"
#include "fnt_medium.h"
#include "fnt_myfont.h"
#include "game_particlesystem.h"
#include "TE_rand.h"
#include "TE_math.h"

typedef struct SpriteData
{
    uint8_t x, y;
    uint8_t width;
    uint8_t height;
    uint8_t pivotX;
    uint8_t pivotY;
} SpriteData;

static const SpriteData _sprites[] = {
    {0, 249, 8, 7, 4, 3},    // fallback
    {2, 64, 10, 10, 5, 4},   // ROBIN_HEAD_FRONT
    {0, 138, 7, 6, 3, 3},    // CART_WHEEL_SIDE
    {0, 127, 16, 10, 8, 4},  // CART_SIDE
    {16, 125, 16, 7, 8, 2},  // CART_GOLD
    {0, 240, 6, 7, 3, 3},    // ARROW_RIGHT
    {161, 48, 4, 7, 2, 4},   // POLE_TOP
    {32, 128, 20, 12, 3, 0}, // ANIM_HAHAHA_R_F1
    {52, 128, 15, 10, 1, 0}, // ANIM_HAHAHA_R_F2
    {68, 128, 9, 7, 0, 0},   // ANIM_HAHAHA_R_F3
    {78, 128, 6, 4, 0, 0},   // ANIM_HAHAHA_R_F4
    {80, 112, 18, 11, 3, 0}, // ANIM_HAHAHA_L_F1
    {82, 123, 13, 13, 8, 0}, // ANIM_HAHAHA_L_F2
    {98, 112, 13, 9, 7, 0},  // ANIM_HAHAHA_L_F3
    {16, 96, 3, 16, 1, 8},   // ANIM_STAFF_HIT_F1
    {20, 97, 11, 13, 5, 6},  // ANIM_STAFF_HIT_F2
    {32, 96, 16, 3, 8, 1},   // ANIM_STAFF_HIT_F3
    {112, 168, 7, 11, 3, 11}, // UI_SHIELD
    {119, 168, 7, 11, 3, 11}, // UI_SWORD
    {112, 179, 13, 5, 6, 0},  // UI_BATTLE_BAR
    {40, 179, 25, 10, 12, 5}, // TEXT_CRIT
    {65, 179, 26, 10, 13, 5}, // TEXT_MISS
    {56, 138, 28, 18, 5, 7}, // TEXT_OUCH
    {84, 138, 21, 13, 10, 6}, // TEXT_OW
    {84, 151, 20, 14, 10, 7}, // TEXT_OOF
    {56, 161, 25, 18, 12, 9}, // TEXT_BANG
    {0, 0, 8, 8, 3, 3},       // TREE_FOLLIAGE_1
    {8, 0, 8, 8, 3, 3},       // TREE_FOLLIAGE_2
    {0, 8, 8, 8, 3, 3},       // TREE_FOLLIAGE_3
    {8, 8, 8, 8, 3, 3},       // TREE_FOLLIAGE_4
    {0, 16, 4, 4, 2, 2},      // TREE_FOLLIAGE_SMALL_1
    {0, 24, 4, 4, 2, 2},      // TREE_FOLLIAGE_SMALL_2
    {8, 16, 4, 4, 2, 2},      // TREE_FOLLIAGE_SMALL_3
    {8, 24, 4, 4, 2, 2},      // TREE_FOLLIAGE_SMALL_4
    {112, 184, 5, 8, 4, 3},   // FLAT_ARROW_RIGHT
    {117, 184, 5, 8, 0, 3},   // FLAT_ARROW_LEFT
    {129, 185, 7, 6, 3, 0},   // FLAT_ARROW_UP
    {122, 185, 7, 6, 3, 5},   // FLAT_ARROW_DOWN
    {112, 192, 6, 7, 0, 0},   // FLAT_ARROW_UP_LEFT
    {118, 192, 6, 7, 5, 0},   // FLAT_ARROW_UP_RIGHT
    {124, 192, 6, 7, 0, 6},   // FLAT_ARROW_DOWN_LEFT
    {130, 192, 6, 7, 5, 6},   // FLAT_ARROW_DOWN_RIGHT
    {248, 0, 7, 8, 4, 4},     // SPRITE_FLAT_ARROW_2_0000
    {248, 8, 7, 8, 4, 4},     // SPRITE_FLAT_ARROW_2_0225
    {248, 16, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_0450
    {248, 24, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_0675
    {248, 32, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_0900
    {248, 40, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_1125
    {248, 48, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_1350
    {248, 56, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_1575
    {248, 64, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_1800
    {248, 72, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_2025
    {248, 80, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_2250
    {248, 88, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_2475
    {248, 96, 7, 8, 4, 4},    // SPRITE_FLAT_ARROW_2_2700
    {248, 104, 7, 8, 4, 4},   // SPRITE_FLAT_ARROW_2_2925
    {248, 112, 7, 8, 4, 4},   // SPRITE_FLAT_ARROW_2_3150
    {248, 120, 7, 8, 4, 4},   // SPRITE_FLAT_ARROW_2_3375
    {16, 136, 12, 6, 6, 3},   // SPRITE_CHARACTER_SHADOW
    {107, 136, 5, 13, 2, 9},  // SPRITE_EXCLAMATION_MARK
    {107, 148, 5, 5, 2, 2},   // SPRITE_TINY_HEART
    {0, 112, 9, 9, 4, 4},     // SPRITE_HEART
    {9, 112, 9, 9, 4, 4},     // SPRITE_HEART_HALF
    {18, 112, 9, 9, 4, 4},    // SPRITE_HEART_EMPTY
    {105, 153, 7, 9, 3, 4},     // SPRITE_HOURGLASS_0
    {105, 161, 7, 9, 3, 4},     // SPRITE_HOURGLASS_1
    {105, 169, 7, 9, 3, 4},     // SPRITE_HOURGLASS_2
    {105, 177, 7, 9, 3, 4},     // SPRITE_HOURGLASS_3
    {105, 185, 7, 9, 3, 4},     // SPRITE_HOURGLASS_4
    {105, 193, 7, 9, 3, 4},     // SPRITE_HOURGLASS_5
    {105, 201, 7, 9, 3, 4},     // SPRITE_HOURGLASS_6
    {98, 166, 7, 9, 3, 4},     // SPRITE_SHIELD
    {112, 200, 14, 15, 3, 1},  // SPRITE_HAND_POINTING_UP
    {248, 128, 8, 8, 4, 4},   // SPRITE_BUTTON_A
    {248, 136, 8, 8, 4, 4},   // SPRITE_BUTTON_B
    {248, 144, 8, 8, 4, 4},   // SPRITE_BUTTON_MENU
    {80, 244, 11, 12, 5, 5},  // SPRITE_EMOJI_FEAR
};

static SpriteData _getSpriteData(uint8_t index)
{
    if (index >= sizeof(_sprites) / sizeof(SpriteData))
        return _sprites[0];
    return _sprites[index];
}

TE_Sprite GameAssets_getSprite(uint8_t index)
{
    SpriteData data = _getSpriteData(index);
    return (TE_Sprite){
        .img = &atlasImg,
        .src = {.x = data.x, .y = data.y, .width = data.width, .height = data.height},
        .pivotX = data.pivotX,
        .pivotY = data.pivotY,
    };
}

#define HAHA_FRAME_DURATION 500
static int DrawAnimation_HAHAHA(TE_Img *dst, uint32_t msTick, uint8_t spriteId, int16_t x, int16_t y, BlitEx blitEx, int loopCount)
{
    // for (int n=0; n<2; n++)
    // {
    msTick *= 2;
    if (msTick + 2 >= (uint32_t)(HAHA_FRAME_DURATION * 5 * loopCount))
        return 0;

    int ftime = msTick % (HAHA_FRAME_DURATION * 5);
    if (ftime > HAHA_FRAME_DURATION * 2)
    {
        uint8_t alpha = 255 - (ftime - HAHA_FRAME_DURATION * 2) * 255 / (HAHA_FRAME_DURATION * 3);
        blitEx.tint = 1;
        blitEx.tintColor = (blitEx.tintColor & 0xffffff) | (alpha << 24);
        blitEx.state.zAlphaBlend = 1;
    }
    int yoff = (-ftime * 2 / HAHA_FRAME_DURATION * 3 - ftime * ftime / (HAHA_FRAME_DURATION * HAHA_FRAME_DURATION * 20)) / 3 + y;
    int fselect = ftime % (HAHA_FRAME_DURATION);
    int animOffset = ftime > HAHA_FRAME_DURATION ? 1 : 0;
    if (fselect < HAHA_FRAME_DURATION / 2)
    {
        TE_Img_blitSprite(dst, GameAssets_getSprite(spriteId + animOffset), x, yoff, blitEx);
    }
    else
    {
        TE_Img_blitSprite(dst, GameAssets_getSprite(spriteId + 1 + animOffset), x + 2, yoff + 3, blitEx);
    }
    return 1;
}

#define STAFF_HIT_FRAME_DURATION 60
static int DrawAnimation_STAFF_HIT(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, int isHit, BlitEx blitEx)
{
    uint32_t frame = msTick / STAFF_HIT_FRAME_DURATION;
    if (frame >= (uint32_t) (6 * maxLoopCount))
        return 0;
    frame %= 6;
    // TE_Debug_drawPixel(x, y, 0xffff00ff);
    if (frame == 2 && isHit)
    {
        // no clue if should do something here
    }
    if (frame == 4 || frame == 3) frame = 2;
    if (frame < 3)
        TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F1 + frame), x, y, blitEx);
    else
    {
        blitEx.flipX = 1;
        blitEx.rotate = 1;
        TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F2), x, y, blitEx);
    }

    return 1;
}

static int DrawAnimation_STAFF_IDLE(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F1), x, y, blitEx);
    return 1;
}

static int DrawAnimation_STAFF_AIM(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    blitEx.rotate = 1;
    TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_ANIM_STAFF_HIT_F1), x, y, blitEx);
    return 1;
}

static int DrawAnimation_HAND_POINTING_UP(TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    float t = msTick * 0.001f;
    y += absi((int16_t)(sinf(t * 5.0f) * 3.0f));
    TE_Img_blitSprite(dst, GameAssets_getSprite(SPRITE_HAND_POINTING_UP), x, y, blitEx);
    return 1;
}

int GameAssets_drawAnimation(uint8_t index, TE_Img *dst, uint32_t msTick, int16_t x, int16_t y, int maxLoopCount, BlitEx blitEx)
{
    switch (index)
    {
    case ANIMATION_HAHAHA_RIGHT:
        return DrawAnimation_HAHAHA(dst, msTick, SPRITE_ANIM_HAHAHA_R_F1, x, y, blitEx, maxLoopCount);
    case ANIMATION_HAHAHA_LEFT:
        return DrawAnimation_HAHAHA(dst, msTick, SPRITE_ANIM_HAHAHA_L_F1, x, y, blitEx, maxLoopCount);
        break;
    case ANIMATION_STAFF_AIM:
        return DrawAnimation_STAFF_AIM(dst, msTick, x, y, maxLoopCount, blitEx);
    case ANIMATION_STAFF_ATTACK_HIT:
        return DrawAnimation_STAFF_HIT(dst, msTick, x, y, maxLoopCount, 1, blitEx);
    case ANIMATION_STAFF_ATTACK:
        return DrawAnimation_STAFF_HIT(dst, msTick, x, y, maxLoopCount, 0, blitEx);
    case ANIMATION_STAFF_IDLE:
        return DrawAnimation_STAFF_IDLE(dst, msTick, x, y, maxLoopCount, blitEx);
    case ANIMATION_HAND_POINTING_UP:
        return DrawAnimation_HAND_POINTING_UP(dst, msTick, x, y, maxLoopCount, blitEx);
    }
    return 0;
}

void GameAssets_drawInputButton(TE_Img *dst, RuntimeContext *ctx, uint16_t button, int16_t x, int16_t y, BlitEx blitEx)
{
    int32_t offset = absi((int32_t)(sinf(ctx->time * 5.0f) * 2.0f));
    TE_Img_fillCircle(dst, x, y, 6, DB32Colors[DB32_BROWN], blitEx.state);
    TE_Img_lineCircle(dst, x, y, 6, DB32Colors[DB32_BLACK], blitEx.state);
    TE_Img_fillCircle(dst, x, y-3 + offset, 6, DB32Colors[DB32_ORANGE], blitEx.state);
    TE_Img_lineCircle(dst, x, y-3 + offset, 6, DB32Colors[DB32_BLACK], blitEx.state);

    TE_Sprite sprite;
    switch (button)
    {
    case INPUT_BUTTON_UP:
        sprite = GameAssets_getSprite(SPRITE_FLAT_ARROW_2_0000);
        break;
    case INPUT_BUTTON_DOWN:
        sprite = GameAssets_getSprite(SPRITE_FLAT_ARROW_2_1800);
        break;
    case INPUT_BUTTON_LEFT:
        sprite = GameAssets_getSprite(SPRITE_FLAT_ARROW_2_0900);
        break;
    case INPUT_BUTTON_RIGHT:
        sprite = GameAssets_getSprite(SPRITE_FLAT_ARROW_2_2700);
        break;
    case INPUT_BUTTON_A:
        sprite = GameAssets_getSprite(SPRITE_BUTTON_A);
        break;
    case INPUT_BUTTON_B:
        sprite = GameAssets_getSprite(SPRITE_BUTTON_B);
        break;
    case INPUT_BUTTON_MENU:
        sprite = GameAssets_getSprite(SPRITE_BUTTON_MENU);
        break;
    
    default:
        return;
    }

    TE_Img_blitSprite(dst, sprite, x + 1, y + offset - 2, blitEx);
}

static TE_Img tinyImg;
static TE_Img mediumImg;
static TE_Img myfontImg;

TE_Font GameAssets_getFont(uint8_t index)
{
    switch (index)
    {
    case FONT_TINY:
    {
        tinyImg = (TE_Img){
            .p2width = fnt_tiny_p2width,
            .p2height = fnt_tiny_p2height,
            .data = (uint32_t *)fnt_tiny_data,
        };
        return (TE_Font){
            .atlas = &tinyImg,
            .glyphCount = fnt_tiny_glyph_count,
            .glyphValues = fnt_tiny_glyphs_values,
            .rectXs = fnt_tiny_glyphs_rects_x,
            .rectYs = fnt_tiny_glyphs_rects_y,
            .rectWidths = fnt_tiny_glyphs_rects_width,
            .rectHeights = fnt_tiny_glyphs_rects_height,
        };
    }
    case FONT_LARGE:
    {
        myfontImg = (TE_Img){
            .p2width = fnt_myfont_p2width,
            .p2height = fnt_myfont_p2height,
            .data = (uint32_t *)fnt_myfont_data,
        };
        return (TE_Font){
            .atlas = &myfontImg,
            .glyphCount = fnt_myfont_glyph_count,
            .glyphValues = fnt_myfont_glyphs_values,
            .rectXs = fnt_myfont_glyphs_rects_x,
            .rectYs = fnt_myfont_glyphs_rects_y,
            .rectWidths = fnt_myfont_glyphs_rects_width,
            .rectHeights = fnt_myfont_glyphs_rects_height,
        };
    }
    default:
    {
        mediumImg = (TE_Img){
            .p2width = fnt_medium_p2width,
            .p2height = fnt_medium_p2height,
            .data = (uint32_t *)fnt_medium_data,
        };
        TE_Font mediumfont = {
            .atlas = &mediumImg,
            .glyphCount = fnt_medium_glyph_count,
            .glyphValues = fnt_medium_glyphs_values,
            .rectXs = fnt_medium_glyphs_rects_x,
            .rectYs = fnt_medium_glyphs_rects_y,
            .rectWidths = fnt_medium_glyphs_rects_width,
            .rectHeights = fnt_medium_glyphs_rects_height,
        };

        return mediumfont;
    }
    }
}

TE_Img *GameAssets_getAtlasImg(void)
{
    return &atlasImg;
}

static uint8_t GameAssets_tryFindSpot(RenderPrefab *prefab, int8_t tries, int16_t x, int16_t y, 
    int8_t scatterX, int8_t scatterY, int8_t rangeTestX, int8_t rangeTestY, uint8_t stepX, uint8_t stepY,
    uint8_t maxFillCount, uint8_t isMinCount, int16_t *outX, int16_t *outY)
{
    for (int i=0;i<tries;i++)
    {
        int16_t px = x + TE_randRange(-scatterX, scatterX);
        int16_t py = y + TE_randRange(-scatterY, scatterY);
        uint8_t fillCount = 0;
        for (int16_t sampleX = px - rangeTestX; sampleX <= px + rangeTestX; sampleX += stepX)
        {
            for (int16_t sampleY = py - rangeTestY; sampleY <= py + rangeTestY; sampleY += stepY)
            {
                if (RenderPrefab_getColorAt(prefab, sampleX, sampleY, 0, 0) != 0)
                {
                    if (++fillCount >= maxFillCount && !isMinCount)
                        goto reject;
                }
            }
        }
        if (isMinCount && fillCount < maxFillCount)
            goto reject;
        *outX = px;
        *outY = py;
        return 1;
        reject:
    }
    return 0;
}

typedef struct TreeSpawnData
{
    TE_Vector2_s8 positions[6];
    uint8_t leafCount;
} TreeSpawnData;

static uint32_t _leafSpawnSeed = 123;
static void GameAssets_treeSpawnLeafs(RuntimeContext *ctx, TE_Img* screen, void *data, int16_t x, int16_t y, int8_t z)
{
    if (!ctx) return;
    uint32_t seed = TE_randSetSeed(_leafSpawnSeed);
    TreeSpawnData *spawnData = (TreeSpawnData*)data;
    if (spawnData->leafCount && TE_randRange(0, 300) < 1)
    {
        uint8_t selectIndex = TE_randRange(0, spawnData->leafCount);
        TE_Vector2_s8 pos = spawnData->positions[selectIndex];
        ParticleSystem_spawn(PARTICLE_TYPE_LEAF, pos.x+ x, pos.y + y, pos.y + 8 + y, 0, 0, (ParticleTypeData){
            .simpleType = {
                .color = DB32Colors[DB32_YELLOW],
                .accelY = 40.0f,
                .drag = 3.0f,
                .maxLife = TE_randRange(10, 100) * 0.05f,
            }
        });
    }
    _leafSpawnSeed = TE_randSetSeed(seed);
}

static RenderPrefab* GameAssets_createTreePrefab(uint16_t id)
{
    uint32_t oldSeed = TE_randSetSeed(id + 2);
    LOG("Creating tree prefab %d", id);
    RenderPrefab *prefab = RenderPrefab_create((RenderObjectCounts){
        .spriteMaxCount = 40,
        .atlasBlitSkewXMaxCount = 1,
        .functionCallMaxCount = 1
    });
    prefab->id = id;

    int8_t x = 0;
    int8_t y = -16;
    
    // Foliage bright green top
    TreeSpawnData *spawnData = RenderObject_malloc(sizeof(TreeSpawnData));
    spawnData->leafCount = 0;
    for (int i=0; i < 7;i++) {
        int16_t px, py;
        if (!GameAssets_tryFindSpot(prefab, 10, x, y, 6, 8, 1, 1, 2, 2, 1, 0, &px, &py)) continue;
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
            .x = px,
            .y = py,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = DB32Colors[30],
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zValue = 8 - py
            }
        });

        // shadows
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
            .x = px - py / 2,
            .y = -py / 2,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = 0x44000000,
                .absZ = 1,
                .state.zValue = 8,
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zAlphaBlend = 1,
            }
        });
            
        spawnData->positions[spawnData->leafCount++] = (TE_Vector2_s8){.x = px, .y = py};
    }

    RenderPrefab_addFunctionCall(prefab, (RenderObjectFunctionCall) {
        .function = GameAssets_treeSpawnLeafs,
        .data = spawnData
    });

    // bright highlights
    for (int i=0; i < 4;i++) {
        int16_t px, py;
        if (!GameAssets_tryFindSpot(prefab, 10, x - 3, y - 5, 6, 6, 1, 1, 2, 2, 3, 1, &px, &py)) continue;
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_SMALL_1, SPRITE_TREE_FOLLIAGE_SMALL_4 + 1),
            .x = px,
            .y = py,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = DB32Colors[8],
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zValue = 13 - py
            }
        });
    }
    x+=1;
    y += 1;
    
    for (int i=0; i < 6;i++) {
        int16_t px, py;
        if (!GameAssets_tryFindSpot(prefab, 10, x, y, 9, 6, 0, 0, 1, 1, 1,0, &px, &py)) continue;
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
            .x = px,
            .y = py,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = DB32Colors[12],
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zValue = 7 - py + TE_randRange(-1,2)
            }
        });
    }

    x+=1;
    y+=2;

    for (int i=0; i < 6;i++) {
        int16_t px, py;
        if (!GameAssets_tryFindSpot(prefab, 10, x, y, 6, 4, 0, 0, 1, 1, 1,0, &px, &py)) continue;
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
            .x = px,
            .y = py,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = DB32Colors[14],
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zValue = 7 - py
            }
        });
    }
    // darkest leafs
    for (int i=0; i < 6;i++) {
        int16_t px, py;
        if (!GameAssets_tryFindSpot(prefab, 10, x, y+2, 8, 6, 0, 0, 2, 2, 3,0, &px, &py)) continue;
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
            .x = px,
            .y = py,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = DB32Colors[1],
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zValue = 7 - py
            }
        });

        // shadows
        RenderPrefab_addSprite(prefab, (RenderObjectSprite){
            .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
            .x = px - py / 2,
            .y = -py / 2,
            .blitEx = {
                .blendMode = TE_BLEND_ALPHAMASK,
                .tint = 1,
                .tintColor = 0x33000000,
                .absZ = 1,
                .state.zValue = 8,
                .state.zCompareMode = Z_COMPARE_LESS,
                .state.zAlphaBlend = 1,
            }
        });
    }

    // Trunk
    RenderPrefab_addAtlasBlitSkewX(prefab, (RenderObjectAtlasBlitSkew) {
        .srcX = 16 * TE_randRange(1,3), .srcY = 48,
        .width = 16, .height = 16,
        .x1 = TE_randRange(-12,-4), .y1 = -12,
        .x2 = -6, .y2 = 4,
        .blitEx = {
            .blendMode = TE_BLEND_ALPHAMASK,
            .state.zCompareMode = Z_COMPARE_LESS,
            .state.zValue = 8
        }
    });

    // x = 6;
    // y = -4;
    // for (int i=0;i<3;i++)
    //     RenderPrefab_addSprite(prefab, (RenderObjectSprite){
    //         .spriteIndex = TE_randRange(SPRITE_TREE_FOLLIAGE_1, SPRITE_TREE_FOLLIAGE_4 + 1),
    //         .x = TE_randRange(-8 + x, 8 + y),
    //         .y = TE_randRange(-8 + x, 8 + y),
    //         .blitEx = {
    //             .blendMode = TE_BLEND_ALPHAMASK,
    //             .tint = 1,
    //             .tintColor = DB32Colors[DB32_DARKGREEN],
    //             .state.zCompareMode = Z_COMPARE_LESS,
    //             .state.zValue = 8
    //         }
    //     });
    
    TE_randSetSeed(oldSeed);
    return prefab;
}

RenderPrefab* GameAssets_getRenderPrefab(uint8_t id, uint8_t variant)
{
    RenderPrefab *prefab = RenderPrefab_getFirstById(id);
    if (prefab)
        return prefab;
    switch (id)
    {
        case RENDER_PREFAB_TREE:
            prefab = RenderPrefab_getFirstById(id << 8 | variant);
            return prefab ? prefab : GameAssets_createTreePrefab(id << 8 | variant);
            
    }
    return prefab;
}