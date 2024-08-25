#include <atlas.h>
#include "TE_rand.h"
#include "game.h"
#include "game_character.h"
#include "game_environment.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Character characters[MAX_ENEMYTYPES];
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

float sqDistF(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

float dotF(float x1, float y1, float x2, float y2)
{
    return x1 * x2 + y1 * y2;
}

float Characters_calcSDFValue(Character *ignore, float x, float y, int16_t *nearestX, int16_t *nearestY)
{
    // instead of transforming every character to world space, let's transform x,y to character space
    // wouldn't work if characters have different base offsets.
    Character_fromBaseF(ignore, &x, &y);
    float nearest = 9999.0f;
    float nearestXF;
    float nearestYF;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].health > 0 && &enemies[i].character != ignore)
        {
            float dx = enemies[i].character.x - x;
            float dy = enemies[i].character.y - y;
            float sqd = dx*dx+dy*dy;
            if (sqd < nearest)
            {
                nearest = sqd;
                nearestXF = enemies[i].character.x;
                nearestYF = enemies[i].character.y;
            }
        }
    }

    if (&playerCharacter != ignore)
    {
        float dx = playerCharacter.x - x;
        float dy = playerCharacter.y - y;
        float sqd = dx*dx+dy*dy;
        if (sqd < nearest)
        {
            nearest = sqd;
            nearestXF = playerCharacter.x;
            nearestYF = playerCharacter.y;
        }
    }

    Character_toBaseF(ignore, &nearestXF, &nearestYF);
    *nearestX = nearestXF;
    *nearestY = nearestYF;

    return sqrtf(nearest);
}

float Obstacles_calcSDFValue(Character *ignore, float x, float y, int16_t *nearestX, int16_t *nearestY)
{
    int16_t nearestEnvX, nearestEnvY;
    int16_t nearestChrX, nearestChrY;
    float sdfEnv = Environment_calcSDFValue(x, y, &nearestEnvX, &nearestEnvY);
    float sdfChr = Characters_calcSDFValue(ignore, x, y, &nearestChrX, &nearestChrY);
    if (sdfEnv < sdfChr)
    {
        *nearestX = nearestEnvX;
        *nearestY = nearestEnvY;
        return sdfEnv;
    }
    *nearestX = nearestChrX;
    *nearestY = nearestChrY;
    return sdfChr;
}

int Characters_raycastCircle(Character* ignore, int x, int y, int radius, int16_t *outCenterX, int16_t *outCenterY, int16_t *outRadius)
{
    int i;
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].health > 0 && &enemies[i].character != ignore)
        {
            if (Character_raycastCircle(&enemies[i].character, x, y, radius, outCenterX, outCenterY, outRadius))
            {
                return i + 1;
            }
        }
    }
    if (&playerCharacter != ignore && Character_raycastCircle(&playerCharacter, x, y, radius, outCenterX, outCenterY, outRadius))
    {
        return 1;
    }
    return 0;
}