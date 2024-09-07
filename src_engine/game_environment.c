#include "game_environment.h"
#include "game.h"
#include "atlas.h"
#include "engine_main.h"
#include "TE_rand.h"
#include "TE_math.h"
#include "game_particlesystem.h"
#include <math.h>

#define TREE_COLLIDE_RADIUS 3

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


#define TYPE_TREE 0
#define TYPE_TREEGROUP 1
#define TYPE_BUSHGROUP 2
#define TYPE_FLOWERGROUP 3

typedef struct TreeGroupData
{
    uint8_t scatterRadius;
    uint8_t count:5;
} TreeGroupData;

typedef struct EnvironmentObject
{
    uint32_t seed;
    int16_t x;
    int16_t y;

    uint8_t type;
    union {
        TreeGroupData treeGroupData;
    };
} EnvironmentObject;

typedef struct EnvironmentScene
{
    EnvironmentObject objects[32];
    int environmentObjectCount;
} EnvironmentScene;

static EnvironmentScene environmentScene;

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

static void DrawBush(TE_Img *img, int16_t bushX, int16_t bushY)
{
    int16_t srcYOffset = 0;
    uint8_t zValue = bushY + 4;
    int16_t x = bushX;
    int16_t y = bushY;
    // TE_Img_fillRect(img, x, y, 8, 8, color, (TE_ImgOpState) {
    //     .zCompareMode = Z_COMPARE_LESS_EQUAL,
    //     .zValue = zValue,
    // });
    uint8_t n = TE_randRange(3, 5);
    for (int i=0;i<n;i++)
    {
        uint32_t color = DB32Colors[i==0 ? 30 : 12 + i%2];

        TE_Img_blitEx(img, &atlasImg, TE_randRange(x-3,x+3), TE_randRange(y-3,y), TE_randRange(0,2)*8, TE_randRange(0,2)*8 + srcYOffset, 8, 8, (BlitEx) {
            .flipX = 0,
            .flipY = 0,
            .rotate = TE_randRange(0, 4),
            .tint = 1,
            .blendMode = TE_BLEND_ALPHAMASK,
            .tintColor = color,
            .state = {
                .zCompareMode = Z_COMPARE_LESS,
                .zValue = zValue,
            }
        });
    }

    // transform x/y to shadow projected x/y
    int dx = x - bushX;
    int dy = y - bushY;
    int shadowX =bushX + 2;
    int shadowY = bushY + 2;
    TE_Img_blitEx(img, &atlasImg, shadowX, shadowY, TE_randRange(0,2)*8, TE_randRange(0,2)*8 + srcYOffset, 8, 8, (BlitEx) {
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

static uint32_t _treeSeed = 1234;
void DrawTree(TE_Img *img, int16_t treeX, int16_t treeY)
{
    treeX += 2;
    treeY += 2;
    TreeNode nodes[256];
    uint8_t pos = 0;
    TreeGen(nodes, &pos, 255, 1, 0, treeX, treeY, TE_randRange(-3,4), -4);
    TE_Img atlasImg = {
        .p2width = atlas_p2width,
        .p2height = atlas_p2height,
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
        { .count = 5, .zValue = 4 + treeY, .compareMode = Z_COMPARE_LESS, .color = 1, .x = 0, .y = -2, .scatterX = 4, .scatterY = 3, .probability = 230, .shadow = 1 },
        { .count = 5, .zValue = 8 + treeY, .compareMode = Z_COMPARE_LESS, .color = 14, .x = -1, .y = -3, .scatterX = 5, .scatterY = 4, .probability = 150, .shadow = 1 },
        { .count = 4, .zValue = 10 + treeY, .compareMode = Z_COMPARE_LESS, .color = 12, .x = -2, .y = -3, .scatterX = 4, .scatterY = 4, .probability = 180, .shadow = 1 },
        { .count = 2, .zValue = 12 + treeY, .compareMode = Z_COMPARE_LESS, .color = 30, .x = -2, .y = -4, .scatterX = 3, .scatterY = 3, .probability = 160 },
        { .count = 2, .zValue = 12 + treeY, .compareMode = Z_COMPARE_EQUAL, .color = 8, .x = -4, .y = -6, .scatterX = 5, .scatterY = 5, .srcYOffset = 16, .probability = 135 },
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
                int zOffset = climbHeight < 4 ? climbHeight * 2 : 8;
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
    
    TE_Vector2_s16 leafdropPoints[32];
    uint32_t leafdropColors[32];
    int leafdropPointCount = 0;

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

                if (leafdropPointCount < 32 && TE_rand() % 256 < 50)
                {
                    leafdropColors[leafdropPointCount] = DB32Colors[instruction->color];
                    leafdropPoints[leafdropPointCount++] = (TE_Vector2_s16) { .x = x, .y = y };
                }

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

    uint32_t oldSeed = TE_randSetSeed(_treeSeed);
    if (leafdropPointCount > 0 && TE_randRange(0, 300) < 1)
    {
        
        int select = TE_rand() % leafdropPointCount;
        TE_Vector2_s16 point = leafdropPoints[select];
        uint32_t color = leafdropColors[select];
        ParticleSystem_spawn(PARTICLE_TYPE_LEAF, point.x, point.y, point.y + 8, 0, 0, (ParticleTypeData){
            .simpleType = {
                .color = color,
                .accelY = 40.0f,
                .drag = 3.0f,
                .maxLife = TE_randRange(10, 100) * 0.05f,
            }
        });
    }
    _treeSeed = TE_randSetSeed(oldSeed);
}

void Environment_init()
{
    environmentScene.environmentObjectCount = 0;
}

void Environment_addTree(int16_t x, int16_t y, uint32_t seed)
{
    environmentScene.objects[environmentScene.environmentObjectCount++] = (EnvironmentObject) {
        .type = TYPE_TREE,
        .seed = seed,
        .x = x,
        .y = y,
    };
}

void Environment_addTreeGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius)
{
    environmentScene.objects[environmentScene.environmentObjectCount++] = (EnvironmentObject) {
        .type = TYPE_TREEGROUP,
        .seed = seed,
        .x = x,
        .y = y,
        .treeGroupData = {
            .count = count,
            .scatterRadius = scatterRadius,
        }
    };
}

void Environment_addBushGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius)
{
    environmentScene.objects[environmentScene.environmentObjectCount++] = (EnvironmentObject) {
        .type = TYPE_BUSHGROUP,
        .seed = seed,
        .x = x,
        .y = y,
        .treeGroupData = {
            .count = count,
            .scatterRadius = scatterRadius,
        }
    };
}

void Environment_addFlowerGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius)
{
    environmentScene.objects[environmentScene.environmentObjectCount++] = (EnvironmentObject) {
        .type = TYPE_FLOWERGROUP,
        .seed = seed,
        .x = x,
        .y = y,
        .treeGroupData = {
            .count = count,
            .scatterRadius = scatterRadius,
        }
    };
}

float Environment_calcSDFValue(int16_t px, int16_t py, int16_t *nearestX, int16_t *nearestY)
{
    uint8_t oldSeed = TE_randGetSeed();
    float minDist = 9999;
    for (int i=0;i<environmentScene.environmentObjectCount;i++)
    {
        TE_randSetSeed(environmentScene.objects[i].seed);
        int x = environmentScene.objects[i].x, y = environmentScene.objects[i].y;
        if (environmentScene.objects[i].type == TYPE_TREE)
        {
            int dx = px - x, dy = py - y;
            float dist = dx * dx + dy * dy;
            if (dist < minDist)
            {
                minDist = dist;
                *nearestX = x;
                *nearestY = y;
            }
        }
        else if (environmentScene.objects[i].type == TYPE_TREEGROUP)
        {
            int posX[32];
            int posY[32];
            int treeGroupCount = environmentScene.objects[i].treeGroupData.count;
            int treeGroupScatterRadius = environmentScene.objects[i].treeGroupData.scatterRadius;
            for (int j=0;j<treeGroupCount;j++)
            {
                TE_randRadius(treeGroupScatterRadius, &posX[j], &posY[j]);
            }
            for (int j=0;j<treeGroupCount;j++)
            {
                int dx = px - x - posX[j], dy = py - y - posY[j];
                float dist = dx * dx + dy * dy;
                if (dist < minDist)
                {
                    minDist = dist;
                    *nearestX = x + posX[j];
                    *nearestY = y + posY[j];
                }
            }
        }
    }
    TE_randSetSeed(oldSeed);
    return sqrtf(minDist) - TREE_COLLIDE_RADIUS;
}

int Environment_raycastCircle(int16_t px, int16_t py, int16_t radius, int16_t *outX, int16_t *outY, int16_t *outRadius)
{
    int dmin = radius + TREE_COLLIDE_RADIUS;
    int dmin2 = dmin * dmin;
    uint32_t oldSeed = TE_randGetSeed();
    for (int i=0;i<environmentScene.environmentObjectCount;i++)
    {
        TE_randSetSeed(environmentScene.objects[i].seed);
        int x = environmentScene.objects[i].x, y = environmentScene.objects[i].y;
        if (environmentScene.objects[i].type == TYPE_TREE)
        {
            int dx = px - x, dy = py - y;
            if (dx * dx + dy * dy < dmin2)
            {
                *outX = x;
                *outY = y;
                *outRadius = TREE_COLLIDE_RADIUS;
                TE_randSetSeed(oldSeed);

                return i;
            }
        }
        else if (environmentScene.objects[i].type == TYPE_TREEGROUP)
        {
            int posX[32];
            int posY[32];
            int treeGroupCount = environmentScene.objects[i].treeGroupData.count;
            int treeGroupScatterRadius = environmentScene.objects[i].treeGroupData.scatterRadius;
            for (int j=0;j<treeGroupCount;j++)
            {
                TE_randRadius(treeGroupScatterRadius, &posX[j], &posY[j]);
            }
            for (int j=0;j<treeGroupCount;j++)
            {
                int dx = px - x - posX[j], dy = py - y - posY[j];
                if (dx * dx + dy * dy < dmin2)
                {
                    *outX = x + posX[j];
                    *outY = y + posY[j];
                    *outRadius = TREE_COLLIDE_RADIUS;

                    TE_randSetSeed(oldSeed);

                    return i;
                }
            }
        }
    }

    TE_randSetSeed(oldSeed);

    return -1;
}
int Environment_raycastPoint(int16_t px, int16_t py)
{
    uint32_t oldSeed = TE_randGetSeed();
    for (int i=0;i<environmentScene.environmentObjectCount;i++)
    {
        TE_randSetSeed(environmentScene.objects[i].seed);
        int x = environmentScene.objects[i].x, y = environmentScene.objects[i].y;
        if (environmentScene.objects[i].type == TYPE_TREE)
        {
            int dx = px - x, dy = py - y;
            if (dx * dx + dy * dy < TREE_COLLIDE_RADIUS * TREE_COLLIDE_RADIUS)
            {
                TE_randSetSeed(oldSeed);
                return i;
            }
        }
        else if (environmentScene.objects[i].type == TYPE_TREEGROUP)
        {
            int posX[32];
            int posY[32];
            int treeGroupCount = environmentScene.objects[i].treeGroupData.count;
            int treeGroupScatterRadius = environmentScene.objects[i].treeGroupData.scatterRadius;
            for (int j=0;j<treeGroupCount;j++)
            {
                TE_randRadius(treeGroupScatterRadius, &posX[j], &posY[j]);
            }
            for (int j=0;j<treeGroupCount;j++)
            {
                int dx = px - x - posX[j], dy = py - y - posY[j];
                if (dx * dx + dy * dy < 6 * 6)
                {
                    TE_randSetSeed(oldSeed);
                    return i;
                }
            }
        }
    }

    TE_randSetSeed(oldSeed);
    return -1;
}

const uint8_t _flowerColor[] = {
    DB32_YELLOW,
    DB32_YELLOW,
    DB32_RED,
    DB32_RED,
    DB32_PINK,
    DB32_PINK,
    DB32_CYAN,
    DB32_WHITE,
};
#define FLOWER_COLOR_COUNT (sizeof(_flowerColor) / sizeof(uint8_t))

void Environment_update(RuntimeContext *ctx, TE_Img* img)
{
    uint32_t oldSeed = TE_randGetSeed();
    for (int i=0;i<environmentScene.environmentObjectCount;i++)
    {
        TE_randSetSeed(environmentScene.objects[i].seed);
        int x = environmentScene.objects[i].x, y = environmentScene.objects[i].y;
        if (environmentScene.objects[i].type == TYPE_TREE)
        {
            DrawTree(img, x, y);
        }
        else if (environmentScene.objects[i].type == TYPE_TREEGROUP)
        {
            int posX[32];
            int posY[32];
            int treeGroupCount = environmentScene.objects[i].treeGroupData.count;
            int treeGroupScatterRadius = environmentScene.objects[i].treeGroupData.scatterRadius;
            for (int j=0;j<treeGroupCount;j++)
            {
                TE_randRadius(treeGroupScatterRadius, &posX[j], &posY[j]);
            }
            for (int j=0;j<treeGroupCount;j++)
            {
                DrawTree(img, x + posX[j], y + posY[j]);
                // TE_Img_lineCircle(img, x + posX[j], y + posY[j], TREE_COLLIDE_RADIUS, DB32Colors[7], (TE_ImgOpState) {
                //     .zCompareMode = Z_COMPARE_LESS,
                //     .zValue = 44 + y + posY[j],
                // });
            }
        }
        else if (environmentScene.objects[i].type == TYPE_BUSHGROUP)
        {
            int posX[32];
            int posY[32];
            int treeGroupCount = environmentScene.objects[i].treeGroupData.count;
            int treeGroupScatterRadius = environmentScene.objects[i].treeGroupData.scatterRadius;
            for (int j=0;j<treeGroupCount;j++)
            {
                TE_randRadius(treeGroupScatterRadius, &posX[j], &posY[j]);
            }
            for (int j=0;j<treeGroupCount;j++)
            {
                DrawBush(img, x + posX[j], y + posY[j]);
            }
        }
        else if (environmentScene.objects[i].type == TYPE_FLOWERGROUP)
        {
            int posX[32];
            int posY[32];
            int treeGroupCount = environmentScene.objects[i].treeGroupData.count;
            int treeGroupScatterRadius = environmentScene.objects[i].treeGroupData.scatterRadius;
            for (int j=0;j<treeGroupCount;j++)
            {
                TE_randRadius(treeGroupScatterRadius, &posX[j], &posY[j]);
            }
            for (int j=0;j<treeGroupCount;j++)
            {
                uint8_t color = _flowerColor[TE_rand() % FLOWER_COLOR_COUNT];
                uint8_t flowerSize = TE_randRange(1, 4);
                uint8_t zValue = 1;
                if (flowerSize == 1)
                {
                    TE_Img_setPixel(img, x + posX[j], y + posY[j], DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                }
                else if(flowerSize == 2)
                {
                    TE_Img_setPixel(img, x + posX[j], y + posY[j], DB32Colors[DB32_YELLOW], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j]+1, y + posY[j], DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j]-1, y + posY[j], DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j], y + posY[j]+1, DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j], y + posY[j]-1, DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                } else if (flowerSize == 3)
                {
                    TE_Img_setPixel(img, x + posX[j], y + posY[j], DB32Colors[DB32_YELLOW], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j]+1, y + posY[j]+1, DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j]-1, y + posY[j]-1, DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j]-1, y + posY[j]+1, DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });
                    TE_Img_setPixel(img, x + posX[j]+1, y + posY[j]-1, DB32Colors[color], (TE_ImgOpState) {
                        .zCompareMode = Z_COMPARE_LESS,
                        .zValue = zValue,
                    });

                }
            }
        }
    }

    TE_randSetSeed(oldSeed);
}