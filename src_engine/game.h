#ifndef __GAME_H__
#define __GAME_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"


typedef struct Player
{
    float x, y;
    float aimX, aimY;
    int dx, dy;
    int dirX, dirY;
    int health;
    int maxHealth;
    uint8_t drawBar;
} Player;

typedef struct Item
{
    int8_t pivotX;
    int8_t pivotY;
    TE_Rect src;
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
    TE_Rect srcHeadFront;
    TE_Rect srcHeadBack;
    TE_Rect srcBodyFront;
    TE_Rect srcBodyBack;
    TE_Rect srcLeftFootFront;
    TE_Rect srcLeftFootBack;
    TE_Rect srcRightHand;
    int8_t itemRightHand;
    int8_t itemLeftHand;
} Character;

typedef struct Projectile
{
    float x, y;
    float vx, vy;
    float nx, ny;
    float lifeTime;
    uint8_t color;
} Projectile;

typedef struct Enemy
{
    float health;
    float idleTime;
    uint8_t id;
    Character character;
} Enemy;

#define MAX_ENEMYTYPES 8
#define PROJECTILE_MAX_COUNT 32
#define MAX_ENEMIES 16

#define ITEM_PIKE 3

extern Character characters[MAX_ENEMYTYPES];
extern Projectile projectiles[PROJECTILE_MAX_COUNT];
extern Enemy enemies[MAX_ENEMIES];

extern Player player;
extern Item items[16];
extern Character playerCharacter;
extern TE_Img atlasImg;

int Characters_raycastCircle(Character* ignore, int x, int y, int radius, int16_t *outCenterX, int16_t *outCenterY, int16_t *outRadius);
float Obstacles_calcSDFValue(Character *ignore, float x, float y, int16_t *nearestX, int16_t *nearestY);

float sqDistF(float x1, float y1, float x2, float y2);
float dotF(float x1, float y1, float x2, float y2);

#endif