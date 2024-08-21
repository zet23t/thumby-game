#include <atlas.h>
#include "TE_rand.h"
#include "game.h"


Character enemyCharacters[MAX_ENEMYTYPES];
Projectile projectiles[PROJECTILE_MAX_COUNT];
Enemy enemies[MAX_ENEMIES];

Player player = {
    .x = 64,
    .y = 64,
    .dx = 0,
    .dy = 0,
    .dirX = 0,
    .dirY = 1,
    .maxHealth = 6,
    .health = 6
};

Item items[16];
Character playerCharacter;

TE_Img atlasImg;

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