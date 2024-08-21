#ifndef __GAME_H__
#define __GAME_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"

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

typedef struct Player
{
    float x, y;
    int dx, dy;
    int dirX, dirY;
    int health;
    int maxHealth;
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
    float targetDistance;
    float speed;
    float lifeTime;
    float shootCooldown;
    int8_t dx, dy;
    int8_t dirX, dirY;
    int8_t isAiming;
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

typedef struct Projectile
{
    float x, y;
    float vx, vy;
    float lifeTime;
    uint8_t color;
} Projectile;

typedef struct Enemy
{
    float health;
    float idleTime;
    Character character;
} Enemy;



#define MAX_ENEMYTYPES 4
#define PROJECTILE_MAX_COUNT 32
#define MAX_ENEMIES 16

extern Character enemyCharacters[MAX_ENEMYTYPES];
extern Projectile projectiles[PROJECTILE_MAX_COUNT];
extern Enemy enemies[MAX_ENEMIES];

extern Player player;
extern Item items[16];
extern Character playerCharacter;
extern TE_Img atlasImg;

void DrawTree(TE_Img *img, int16_t treeX, int16_t treeY);
void TreeGen(TreeNode *nodes, uint8_t *pos, uint8_t maxCount, uint8_t isMain, uint8_t depth, int8_t x, int8_t y, int8_t dx, int8_t dy);


#endif