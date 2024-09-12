#ifndef __GAME_H__
#define __GAME_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"
#include "TE_debug.h"


typedef struct Player
{
    float x, y;
    float aimX, aimY;
    int dx, dy;
    int dirX, dirY;
    int health;
    int maxHealth;
    uint8_t drawBar:1;
    uint8_t attackQuality:2;
    uint8_t defenseQuality:2;
    uint8_t defenseQualityResult:2;
    float defenseActionStep[4];
    float aimTimer;
    float defTimer;
} Player;


typedef struct Item
{
    int8_t pivotX;
    int8_t pivotY;
    int8_t idleAnimationId;
    int8_t aimAnimationId;
    int8_t attackAnimationId;
    int8_t hitAnimationId;
    uint8_t meleeRange:4;
    float cooldown;
    TE_Rect src;
} Item;

typedef struct Character 
{
    float x, y;
    float prevX, prevY;
    float targetX, targetY;
    float walkDistance;
    float targetDistance;
    float speed;
    float lifeTime;
    float shootCooldown;
    float runningAnimationTime;
    int8_t dx, dy;
    int8_t dirX, dirY;
    int8_t maskDir:1;
    int8_t isAiming:1;
    int8_t isStriking:1;
    int8_t isHitting:1;
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

typedef struct Enemy Enemy;
typedef struct EnemyCallbackArg {
    uint8_t type;
    union {
        struct {
            float damage, vx, vy;
        } tookDamage;
    };
} EnemyCallbackArg;

#define ENEMY_CALLBACK_TYPE_TOOK_DAMAGE 1
#define ENEMY_CALLBACK_TYPE_UPDATE 2
typedef void(*EnemyCallbackFn)( struct Enemy *enemy, EnemyCallbackArg data, RuntimeContext *ctx, TE_Img *screen);
typedef struct EnemyCallbackUserData
{
    EnemyCallbackFn callback;
    void *dataPointer;
} EnemyCallbackUserData;


typedef struct Enemy
{
    float health;
    float idleTime;
    uint8_t id;
    Character character;
    EnemyCallbackUserData userCallbackData;
} Enemy;

#define MAX_ENEMYTYPES 8
#define PROJECTILE_MAX_COUNT 32
#define MAX_ENEMIES 16

#define ITEM_PIKE 3
#define ITEM_STAFF 4

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