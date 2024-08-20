#include "engine_main.h"
#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include <atlas.c>
#include <fnt_myfont.c>
#include <fnt_tiny.c>

typedef struct TE_Img
{
    uint8_t p2width;
    uint8_t p2height;
    uint32_t *data;
} TE_Img;

typedef struct TE_Font
{
    TE_Img *atlas;
    uint16_t glyphCount;
    const uint16_t *glyphValues;
    const uint16_t *rectXs;
    const uint16_t *rectYs;
    const uint8_t *rectWidths;
    const uint8_t *rectHeights;
} TE_Font;


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

void TE_Img_setPixel(TE_Img *img, uint16_t x, uint16_t y, uint32_t color, TE_ImgOpState state)
{
    if (x >= (1 << img->p2width) || y >= (1 << img->p2height))
    {
        return;
    }
    uint32_t *pixel = &img->data[(y << img->p2width) + x];
    uint8_t zDst = *pixel >> 24;
    if ((state.zCompareMode == Z_COMPARE_ALWAYS) ||
        (state.zCompareMode == Z_COMPARE_EQUAL && zDst == state.zValue) ||
        (state.zCompareMode == Z_COMPARE_LESS && zDst < state.zValue) ||
        (state.zCompareMode == Z_COMPARE_GREATER && zDst > state.zValue) ||
        (state.zCompareMode == Z_COMPARE_LESS_EQUAL && zDst <= state.zValue) ||
        (state.zCompareMode == Z_COMPARE_GREATER_EQUAL && zDst >= state.zValue) ||
        (state.zCompareMode == Z_COMPARE_NOT_EQUAL && zDst != state.zValue)
    )
    {
        if (state.zNoWrite)
        {
            color = (color & 0xffffff) | (zDst << 24);
        }
        else
        {
            color = (color & 0xffffff) | (state.zValue << 24);
        }
        *pixel = color;
    }
}

// void TE_Img_blit(TE_Img *img, TE_Img *src, uint16_t x, uint16_t y, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height)
// {
//     for (uint16_t i = 0; i < width; i++)
//     {
//         for (uint16_t j = 0; j < height; j++)
//         {
//            TE_Img_setPixel(img, x + i, y + j, src->data[((srcY + j) << src->p2width) + srcX + i]);
//         }
//     }
// }

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

uint32_t TE_Img_getPixelEx(TE_Img *img, uint16_t ox, uint16_t oy, uint16_t x, uint16_t y, uint16_t w, uint16_t h, BlitEx options)
{
    if (options.flipX)
    {
        x = w - x - 1;
    }
    if (options.flipY)
    {
        y = h - y - 1;
    }
    if (options.rotate == 1)
    {
        uint16_t tmp = x;
        x = y;
        y = w - tmp - 1;
    }
    else if (options.rotate == 2)
    {
        x = w - x - 1;
        y = h - y - 1;
    }
    else if (options.rotate == 3)
    {
        uint16_t tmp = x;
        x = h - y - 1;
        y = tmp;
    }

    x += ox;
    y += oy;

    return img->data[(y << img->p2width) + x];
}

int abs(int x)
{
    return x < 0 ? -x : x;
}

void TE_Img_line(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, TE_ImgOpState state)
{
    int16_t dx = abs(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (1)
    {
        TE_Img_setPixel(img, x0, y0, color, state);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        int16_t e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

uint32_t TE_Color_tint(uint32_t color, uint32_t tint)
{
    uint32_t r = ((color & 0xFF) * (tint & 0xFF)) >> 8;
    uint32_t g = (((color >> 8) & 0xFF) * ((tint >> 8) & 0xFF)) >> 8;
    uint32_t b = (((color >> 16) & 0xFF) * ((tint >> 16) & 0xFF)) >> 8;
    uint32_t a = (((color >> 24) & 0xFF) * ((tint >> 24) & 0xFF)) >> 8;
    return r | (g << 8) | (b << 16) | (a << 24);
}

void TE_Img_blitEx(TE_Img *img, TE_Img *src, int16_t x, int16_t y, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height, BlitEx options)
{
    for (uint16_t i = 0; i < width; i++)
    {
        for (uint16_t j = 0; j < height; j++)
        {
            uint32_t color = TE_Img_getPixelEx(src, srcX, srcY, i, j, width, height, options);
            if (options.tint)
            {
                color = TE_Color_tint(color, options.tintColor);
            }

            if (options.blendMode == TE_BLEND_ALPHAMASK)
            {
                if ((color & 0xFF000000) == 0)
                {
                    continue;
                }
            }

            TE_Img_setPixel(img, x + i, y + j, color, options.state);
        }
    }
}

void TE_Img_clear(TE_Img *img, uint32_t color, uint8_t z)
{
    color = (color & 0xffffff) | (z << 24);
    for (uint32_t i = 0; i < (1 << img->p2width) * (1 << img->p2height); i++)
    {
        img->data[i] = color;
    }
}

int TE_seed = 0x3291;
uint32_t TE_rand()
{
    TE_seed = TE_seed * 1103515245 + 12345;
    return (TE_seed / 65536) % 32768;
}

int32_t TE_randRange(int32_t min, int32_t max)
{
    return min + TE_rand() % (max - min);
}

int TE_Font_drawChar(TE_Img *img, TE_Font *font, uint16_t x, uint16_t y, char c, uint32_t color, TE_ImgOpState state)
{
    for (uint16_t i = 0; i < font->glyphCount; i++)
    {
        if (font->glyphValues[i] == c)
        {
            TE_Img_blitEx(img, font->atlas, x, y, font->rectXs[i], font->rectYs[i], font->rectWidths[i], font->rectHeights[i], (BlitEx) {
                .flipX = 0,
                .flipY = 0,
                .rotate = 0,
                .tint = 1,
                .blendMode = TE_BLEND_ALPHAMASK,
                .tintColor = color,
                .state = state,
            });
            return font->rectWidths[i];
        }
    }
    return 0;
}

int TE_Font_drawText(TE_Img *img, TE_Font *font, uint16_t x, uint16_t y, int8_t spacing, const char *text, uint32_t color, TE_ImgOpState state)
{
    int width = 0;
    while (*text)
    {
        width += TE_Font_drawChar(img, font, x + width, y, *text, color, state) + spacing;
        text++;
    }
    return width;
}

static int counter = 0;

uint32_t DB32Colors[] = {
    0xFF000000, 0xFF342022, 0xFF3C2845, 0xFF313966, 0xFF3B568F, 0xFF2671DF, 0xFF66A0D9, 0xFF9AC3EE,
    0xFF36F2FB, 0xFF50E599, 0xFF30BE6A, 0xFF6E9437, 0xFF2F694B, 0xFF244B52, 0xFF393C32, 0xFF743F3F,
    0xFF826030, 0xFFE16E5B, 0xFFFF9B63, 0xFFE4CD5F, 0xFFFCDBCB, 0xFFFFFFFF, 0xFFB7AD9B, 0xFF877E84,
    0xFF6A6A69, 0xFF525659, 0xFF8A4276, 0xFF3232AC, 0xFF6357D9, 0xFFBA7BD7, 0xFF4A978F, 0xFF306F8A,
    0xFF36535E, 0xFF48687D, 0xFF3C7EA0, 0xFFC7C3C2, 0xFFE0E0E0,
};

typedef struct TreeDrawInstruction
{
    int16_t x, scatterX;
    int16_t y, scatterY;
    uint8_t color;
    uint8_t count;
    uint8_t probability;
    uint8_t srcYOffset;
    uint8_t zValue;
    uint8_t compareMode;
    uint8_t shadow;
} TreeDrawInstruction;

typedef struct TreeNode
{
    uint8_t depth;
    uint8_t isMain;
    int16_t x1, y1, x2, y2;
} TreeNode;

void TreeGen(TreeNode *nodes, uint8_t *pos, uint8_t maxCount, uint8_t isMain, uint8_t depth, int8_t x, int8_t y, int8_t dx, int8_t dy)
{
    if (*pos >= maxCount || depth > 4)
    {
        return;
    }
    int length = (dx * dx + dy * dy) >> 2;
    if (length < 1)
    {
        return;
    }
    if (TE_rand() % 256 < 150)
    {
        dx = dx > 0 ? dx - 1 : dx + 1;
    }
    if (TE_rand() % 256 < 70)
    {
        dy = dy - 1;
    }
    int8_t x2 = x + dx + TE_randRange(-1, 2);
    int8_t y2 = y + dy + TE_randRange(-1, 2);
    nodes[(*pos)++] = (TreeNode) {
        .depth = depth,
        .x1 = x,
        .y1 = y,
        .x2 = x2,
        .y2 = y2,
        .isMain = isMain,
    };

    TreeGen(nodes, pos, maxCount, isMain, depth + 1, x2, y2, dx, dy);
    if (depth > 0 && (TE_rand() % 256 < 150 || depth == 2))
    {
        int8_t dx2 = dx < 0 ? TE_randRange(1,5) : TE_randRange(-5,-1);
        TreeGen(nodes, pos, maxCount, 0, depth + 1, x2, y2, dx2, dy * 3 / 5);
    }
}

void DrawTree(TE_Img *img, int16_t treeX, int16_t treeY)
{
    TreeNode nodes[256];
    uint8_t pos = 0;
    TreeGen(nodes, &pos, 255, 1, 0, treeX, treeY, TE_randRange(-3,4), -4);
    TE_Img atlasImg = {
        .p2width = 7,
        .p2height = 7,
        .data = (uint32_t*) atlas_data,
    };
    int8_t treePoints[16*2];
    int treePointCount = 0;
    for (int i=0;i<pos-2 && treePointCount < 16;i++)
    {
        if (treePointCount > 0 && TE_rand() % 256 < 100)
        {
            continue;
        }
        int x = nodes[i+2].x2;
        int y = nodes[i+2].y2;
        for (int j=0;j<treePointCount;j++)
        {
            int dx = treePoints[j * 2] - x;
            int dy = treePoints[j * 2 + 1] - y;
            if (dx * dx + dy * dy < 8)
            {
                goto reject;
            }
        }
        treePoints[treePointCount * 2] = x;
        treePoints[treePointCount * 2 + 1] = y;
        treePointCount++;
        reject:
    }

    TreeDrawInstruction instructions[] = {
        { .count = 5, .zValue = 2 + treeY, .compareMode = Z_COMPARE_LESS, .color = 1, .x = 0, .y = -2, .scatterX = 4, .scatterY = 3, .probability = 230, .shadow = 1 },
        { .count = 5, .zValue = 4 + treeY, .compareMode = Z_COMPARE_LESS, .color = 14, .x = -1, .y = -3, .scatterX = 5, .scatterY = 4, .probability = 150, .shadow = 1 },
        { .count = 4, .zValue = 5 + treeY, .compareMode = Z_COMPARE_LESS, .color = 12, .x = -2, .y = -3, .scatterX = 4, .scatterY = 4, .probability = 180, .shadow = 1 },
        { .count = 2, .zValue = 6 + treeY, .compareMode = Z_COMPARE_LESS, .color = 30, .x = -2, .y = -4, .scatterX = 3, .scatterY = 3, .probability = 160 },
        { .count = 2, .zValue = 6 + treeY, .compareMode = Z_COMPARE_EQUAL, .color = 8, .x = -4, .y = -6, .scatterX = 5, .scatterY = 5, .srcYOffset = 16, .probability = 135 },
    };

    int climbHeight = 0;
    // drawing the trunk
    for (int i=0;i<pos;i++)
    {
        TreeNode *node = &nodes[i];
        if (node->isMain)
        {
            int dx = node->x2 - node->x1;
            int dy = node->y2 - node->y1;
            for (int cy = node->y1; cy > node->y2; cy--)
            {
                int shiftX = (cy - node->y1) * dx / dy;
                int zOffset = climbHeight < 4 ? climbHeight : 4;
                int x = node->x1 - 8 + shiftX;
                int y = cy;
                TE_Img_blitEx(img, &atlasImg, x, y, 32, 63 - climbHeight, 16, 1,
                    (BlitEx) {
                        .flipX = 0,
                        .flipY = 0,
                        .rotate = 0,
                        .tint = 0,
                        .blendMode = TE_BLEND_ALPHAMASK,
                        .state = {
                            .zCompareMode = Z_COMPARE_LESS_EQUAL,
                            .zValue = zOffset + treeY,
                        }
                    });
                
                int dx = x - treeX;
                int dy = y - treeY;
                int shadowX =(dx - dy) / 4 + treeX - 1;
                int shadowY = -dy / 8 + treeY - 2;
                TE_Img_line(img, shadowX, shadowY, shadowX + 6, shadowY, DB32Colors[14], (TE_ImgOpState) {
                    .zCompareMode = Z_COMPARE_EQUAL,
                    .zValue = 0,
                });
                if (climbHeight < 15) climbHeight++;
                else climbHeight -= 4;
            }
        }
        // TE_Img_line(img, node->x1, node->y1, node->x2, node->y2, DB32Colors[2]);
    }

    // drawing the leafs
    for (int td = 0; td < sizeof(instructions) / sizeof(TreeDrawInstruction); td++)
    {
        TreeDrawInstruction *instruction = &instructions[td];
        int32_t minX = instruction->x - instruction->scatterX;
        int32_t maxX = instruction->x + instruction->scatterX;
        int32_t minY = instruction->y - instruction->scatterY;
        int32_t maxY = instruction->y + instruction->scatterY;
        for (int i=0;i<treePointCount;i++)
        {
            for (int j=0;j<instruction->count;j++)
            {
                if (TE_rand() % 256 > instruction->probability)
                {
                    continue;
                }
                int x = treePoints[i * 2] + TE_randRange(minX, maxX);
                int y = treePoints[i * 2 + 1] + TE_randRange(minY, maxY);
                
                TE_Img_blitEx(img, &atlasImg, x, y, TE_randRange(0,2)*8, TE_randRange(0,2)*8 + instruction->srcYOffset, 8, 8, (BlitEx) {
                    .flipX = 0,
                    .flipY = 0,
                    .rotate = TE_randRange(0, 4),
                    .tint = 1,
                    .blendMode = TE_BLEND_ALPHAMASK,
                    .tintColor = DB32Colors[instruction->color],
                    .state = {
                        .zCompareMode = instruction->compareMode,
                        .zValue = instruction->zValue,
                    }
                });

                if (!instruction->shadow)
                {
                    continue;
                }
                // transform x/y to shadow projected x/y
                int dx = x - treeX;
                int dy = y - treeY;
                int shadowX =(dx - dy) / 4 + TE_randRange(-4,8) + treeX;
                int shadowY = -dy / 8 + TE_randRange(-4,8) + treeY;
                TE_Img_blitEx(img, &atlasImg, shadowX, shadowY, TE_randRange(0,2)*8, TE_randRange(0,2)*8 + instruction->srcYOffset, 8, 8, (BlitEx) {
                    .flipX = 0,
                    .flipY = 0,
                    .rotate = TE_randRange(0, 4),
                    .tint = 1,
                    .blendMode = TE_BLEND_ALPHAMASK,
                    .tintColor = DB32Colors[14],
                    .state = {
                        .zCompareMode = Z_COMPARE_EQUAL,
                        .zValue = 0,
                    }
                });
            }
        }
    }

    
}

typedef struct Player
{
    float x, y;
    int dx, dy;
    int dirX, dirY;
} Player;

typedef struct Item
{
    int8_t pivotX;
    int8_t pivotY;
    TL_Rect src;
} Item;

typedef struct Character 
{
    float x, y;
    float targetX, targetY;
    float walkDistance;
    float speed;
    float lifeTime;
    int8_t dx, dy;
    int8_t dirX, dirY;
    TL_Rect srcHeadFront;
    TL_Rect srcHeadBack;
    TL_Rect srcBodyFront;
    TL_Rect srcBodyBack;
    TL_Rect srcLeftFootFront;
    TL_Rect srcLeftFootBack;
    TL_Rect srcRightHand;
    int8_t itemRightHand;
    int8_t itemLeftHand;
} Character;

Player player = {
    .x = 64,
    .y = 64,
    .dx = 0,
    .dy = 0,
    .dirX = 0,
    .dirY = 1,
};

Item items[16];
Character playerCharacter;

TE_Img atlasImg;

DLL_EXPORT void init()
{
    atlasImg = (TE_Img) {
        .p2width = atlas_p2width,
        .p2height = atlas_p2height,
        .data = (uint32_t*) atlas_data,
    };

    items[0] = (Item) {
        .pivotX = 2,
        .pivotY = 4,
        .src = { .x = 0, .y = 96, .width = 8, .height = 13 },
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
}

#define RECTARG(r) r.x, r.y, r.width, r.height

void Character_update(Character *character, RuntimeContext *ctx, TE_Img *img, float tx, float ty, int8_t dirX, int8_t dirY)
{
    float dx = tx - character->x;
    float dy = ty - character->y;
    int8_t signX = dx < 0.0f ? -1 : (dx > 0.0f ? 1 : 0);
    int8_t signY = dy < 0.0f ? -1 : (dy > 0.0f ? 1 : 0);
    float len = sqrtf(dx * dx + dy * dy);
    character->lifeTime += ctx->deltaTime;
    character->walkDistance += len;
    character->speed = len / ctx->deltaTime;
    character->x = tx;
    character->y = ty;
    character->dirX = dirX;
    character->dirY = dirY;

    uint8_t walkPhase = 0;
    uint8_t walkPhase1 = 0;
    uint8_t walkPhase2 = 0;
    if (character->speed > 0.0f)
    {
        walkPhase = (int)(character->lifeTime * 10) % 2;
        walkPhase1 = (int)(character->lifeTime * 10 + 2.5f) % 2;
        walkPhase2 = (int)(character->lifeTime * 5 + .5f) % 2;
    }

    uint8_t charZ = (uint8_t) character->y + 12;
    int16_t x = (int16_t) floorf(character->x);
    int16_t y = (int16_t) floorf(character->y);
    TL_Rect srcHead = character->dirY < 0 ? character->srcHeadBack : character->srcHeadFront;
    TE_Img_blitEx(img, &atlasImg, x - 8, y - 6, RECTARG(srcHead), (BlitEx) {
        .flipX = character->dirX < 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 8,
        }
    });

    TL_Rect srcBody = character->dirY < 0 ? character->srcBodyBack : character->srcBodyFront;
    TE_Img_blitEx(img, &atlasImg, x - 8, y - 8 + 10 - walkPhase2, RECTARG(srcBody), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 7,
        }
    });

    // feet
    TL_Rect srcLeftFoot = character->dirY < 0 ? character->srcLeftFootBack : character->srcLeftFootFront;
    TE_Img_blitEx(img, &atlasImg, x - 8, y - 8 + 14 - walkPhase, 
        RECTARG(srcLeftFoot), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 6,
        }
    });
    TE_Img_blitEx(img, &atlasImg, x - 2, y - 8 + 14 - (walkPhase1) % 2, 
        RECTARG(srcLeftFoot), (BlitEx) {
        .flipX = 1,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 6,
        }
    });

    // hands
    
    int16_t rHandX = x - 2 + 2 + (dirX < 0 ? 3 : 1) * dirX;
    int16_t rHandY = y - 8 + 11 + dirY;
    int16_t lHandX = x - 2 - 7 + (dirX < 0 ? 1 : 3) * dirX;
    int16_t lHandY = y - 8 + 11 + dirY;
    TE_Img_blitEx(img, &atlasImg, rHandX, rHandY, RECTARG(character->srcRightHand), (BlitEx) {
        .flipX = 0,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 7 + dirY,
        }
    });
    TE_Img_blitEx(img, &atlasImg, lHandX, lHandY, RECTARG(character->srcRightHand), (BlitEx) {
        .flipX = 1,
        .flipY = 0,
        .rotate = 0,
        .tint = 0,
        .blendMode = TE_BLEND_ALPHAMASK,
        .tintColor = 0xffffffff,
        .state = {
            .zCompareMode = Z_COMPARE_LESS,
            .zValue = charZ + 7 + dirY,
        }
    });

    // items
    int itemRight = dirY < 0 ? character->itemLeftHand : character->itemRightHand;
    int itemLeft = dirY < 0 ? character->itemRightHand : character->itemLeftHand;
    if (itemRight != 0)
    {
        uint8_t id = itemRight < 0 ? -itemRight : itemRight;
        Item *item = &items[id - 1];
        int8_t pivotDir = dirX | 1;
        TE_Img_blitEx(img, &atlasImg, 
            rHandX + dirX - item->pivotX * pivotDir, rHandY - item->pivotY, 
            RECTARG(item->src), (BlitEx) {
            .flipX = (itemRight < 0) ^ (dirX < 0),
            .flipY = 0,
            .rotate = 0,
            .tint = 0,
            .blendMode = TE_BLEND_ALPHAMASK,
            .tintColor = 0xffffffff,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = charZ + 7 + dirY * 2,
            }
        });
    }

    if (itemLeft != 0)
    {
        uint8_t id = itemLeft < 0 ? -itemLeft : itemLeft;
        Item *item = &items[id - 1];
        int8_t pivotDir = dirX | 1;
        TE_Img_blitEx(img, &atlasImg, 
            lHandX + dirX - item->pivotX * pivotDir, lHandY - item->pivotY, 
            RECTARG(item->src), (BlitEx) {
            .flipX = (itemLeft < 0) ^ (dirX < 0),
            .flipY = 0,
            .rotate = 0,
            .tint = 0,
            .blendMode = TE_BLEND_ALPHAMASK,
            .tintColor = 0xffffffff,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = charZ + 7 + dirY * 2,
            }
        });
    }
}

DLL_EXPORT void update(RuntimeContext *ctx)
{
    if (ctx->inputUp || ctx->inputDown)
    {
        player.dirX = player.dirY = 0;
    }

    if (ctx->inputUp)
    {
        player.dy = -1;
        player.dirY = -1;
    }
    else if (ctx->inputDown)
    {
        player.dy = 1;
        player.dirY = 1;
    }
    else
    {
        player.dy = 0;
    }

    if (ctx->inputLeft)
    {
        player.dx = -1;
        player.dirX = -1;
    }
    else if (ctx->inputRight)
    {
        player.dx = 1;
        player.dirX = 1;
    }
    else
    {
        player.dx = 0;
    }

    int sqLen = player.dx * player.dx + player.dy * player.dy;
    float multiplier = sqLen == 2 ? 0.70710678118f * 16.0f : 16.0f;
    if (sqLen == 2)
    {
        float frac = player.x - (int)player.x;
        player.y = (int)player.y + frac;
    }

    player.x += player.dx * ctx->deltaTime * multiplier;
    player.y += player.dy * ctx->deltaTime * multiplier;

    TE_Img img = {
        .p2width = 7,
        .p2height = 7,
        .data = ctx->screenData,
    };

    TE_Img myFontImg = {
        .p2width = fnt_myfont_p2width,
        .p2height = fnt_myfont_p2height,
        .data = (uint32_t*) fnt_myfont_data,
    };
    
    TE_Img tinyImg = {
        .p2width = fnt_tiny_p2width,
        .p2height = fnt_tiny_p2height,
        .data = (uint32_t*) fnt_tiny_data,
    };

    TE_Font myfont = {
        .atlas = &myFontImg,
        .glyphCount = fnt_myfont_glyph_count,
        .glyphValues = fnt_myfont_glyphs_values,
        .rectXs = fnt_myfont_glyphs_rects_x,
        .rectYs = fnt_myfont_glyphs_rects_y,
        .rectWidths = fnt_myfont_glyphs_rects_width,
        .rectHeights = fnt_myfont_glyphs_rects_height,
    };

    TE_Font tinyfont = {
        .atlas = &tinyImg,
        .glyphCount = fnt_tiny_glyph_count,
        .glyphValues = fnt_tiny_glyphs_values,
        .rectXs = fnt_tiny_glyphs_rects_x,
        .rectYs = fnt_tiny_glyphs_rects_y,
        .rectWidths = fnt_tiny_glyphs_rects_width,
        .rectHeights = fnt_tiny_glyphs_rects_height,
    };

    counter++;
    
    TE_Img_clear(&img, DB32Colors[13], 0);

    TE_seed = 3294;
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
    DrawTree(&img, 30,50);
    // DrawTree(&img, 35,50);
    // DrawTree(&img, 25,30);
    DrawTree(&img, 60,50);
    DrawTree(&img, 90,50);
    DrawTree(&img, 35,90);
    DrawTree(&img, 65,110);
    DrawTree(&img, 90,80);
    DrawTree(&img, 95,80);
    DrawTree(&img, 95,90);
    // DrawTree(&img, 30,80);
    // DrawTree(&img, 60,80);
    // DrawTree(&img, 90,80);

    Character_update(&playerCharacter, ctx, &img, player.x, player.y, player.dirX, player.dirY);
    
    

    TE_Font_drawText(&img, &myfont, 2, 2, -1, "Sherwood Forest", 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 100,
    });

    char text[64];
    sprintf(text, "FPS: %d", (int)(1.0f / ctx->deltaTime));
    TE_Font_drawText(&img, &tinyfont, 2, 112, -1, text, 0xffffffff, (TE_ImgOpState) {
        .zCompareMode = Z_COMPARE_LESS_EQUAL,
        .zValue = 100,
    });
}