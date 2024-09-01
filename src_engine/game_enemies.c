#include "game_enemies.h"
#include "game_character.h"
#include "game_environment.h"
#include "TE_rand.h"
#include <math.h>
#include <stdio.h>

void Enemies_init()
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        enemies[i] = (Enemy) {
            .health = 0.0f,
            .character = characters[0],
        };
    }
}

void Enemies_setItem(uint8_t id, int8_t leftItemIndex, int8_t rightItemIndex)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f && enemies[i].id == id)
        {
            enemies[i].character.itemLeftHand = leftItemIndex;
            enemies[i].character.itemRightHand = rightItemIndex;
            break;
        }
    }
}

int Enemies_spawn(uint8_t id, int type, int16_t x, int16_t y)
{
    int index = -1;
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health <= 0.0f)
        {
            index = i;
            break;
        }
    }
    if (index < 0)
    {
        return -1;
    }
    enemies[index] = (Enemy) {
        .health = 3.0f,
        .character = characters[type],
    };
    enemies[index].id = id;
    enemies[index].character.x = x;
    enemies[index].character.y = y;
    enemies[index].character.dirY = 1;
    enemies[index].character.targetX = x;
    enemies[index].character.targetY = y;
    return index;
}

void Enemies_setTarget(uint8_t id, float x, float y)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f && enemies[i].id == id)
        {
            enemies[i].character.targetX = x;
            enemies[i].character.targetY = y;
            break;
        }
    }
}

void Enemies_update(RuntimeContext *ctx, TE_Img *img)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f)
        {
            float targetX = enemies[i].character.targetX;
            float targetY = enemies[i].character.targetY;
            float x = enemies[i].character.x;
            float y = enemies[i].character.y;
            Character_toBaseF(&enemies[i].character, &targetX, &targetY);
            Character_toBaseF(&enemies[i].character, &x, &y);
            float dx = targetX - x;
            float dy = targetY - y;
            float sqDist = dx*dx+dy*dy;
            if (sqDist > 10.0f)
            {
                int16_t nearestX, nearestY;
                float dist = sqrtf(sqDist);
                dx /= dist;
                dy /= dist;
                float sdf = Obstacles_calcSDFValue(&enemies[i].character, (int16_t) (x + dx * 3.0f), (int16_t) (y + dy * 3.0f), &nearestX, &nearestY);
                if (sdf < 12.0f && dotF(nearestX - x, nearestY - y, dx, dy) > 0.0f)
                {
                    float deflectX = (x + dx * 3.0f) - nearestX;
                    float deflectY = (y + dy * 3.0f) - nearestY;

                    // TE_Img_line(img, x + dx * 3.0f, y + dy * 3.0f, nearestX, nearestY, 0xff0000ff, (TE_ImgOpState){.zValue = 255});
                    // TE_Img_line(img, x, y, x + deflectX, y + deflectY, 0xffff0000, (TE_ImgOpState){.zValue = 255});

                    float deflectLength = sqrtf(deflectX * deflectX + deflectY * deflectY);
                    if (deflectLength > 0.0f)
                    {
                        int side = dx * deflectY - dy * deflectX > 0.0f ? 1 : -1;
                        dx += deflectY / deflectLength * side;
                        dy -= deflectX / deflectLength * side;
                    }
                }
                dx *= 5.0f;
                dy *= 5.0f;
                
            }
            
            // TE_Img_line(img, x, y, x + dx, y + dy, 0xff00ff00, (TE_ImgOpState){.zValue = 255});
            // TE_Img_line(img, x, y, targetX, targetY, 0xff00ffff, (TE_ImgOpState){.zValue = 255});

            Character_fromBaseF(&enemies[i].character, &targetX, &targetY);
            Character_fromBaseF(&enemies[i].character, &x, &y);
            Character_update(&enemies[i].character, ctx, img, x + dx, y + dy, enemies[i].character.dirX, enemies[i].character.dirY);
            enemies[i].character.targetX = targetX;
            enemies[i].character.targetY = targetY;

            // TE_Img_lineRect(img, enemies[i].character.x + ENEMY_RECT_X, enemies[i].character.y + ENEMY_RECT_Y, ENEMY_RECT_WIDTH, ENEMY_RECT_HEIGHT, 0xff00FF00, (TE_ImgOpState){0});
            // if (sqDistF(x, y, targetX, targetY) < 2.0f)
            // {
            //     enemies[i].idleTime += ctx->deltaTime;
            //     if (enemies[i].idleTime > 2.0f)
            //     {
            //         uint32_t seed = (uint32_t) (enemies[i].character.x * 100.0f) + (uint32_t) (enemies[i].character.y * 1000.0f);
            //         TE_randSetSeed(seed);
            //         while(1)
            //         {
            //             enemies[i].character.targetX = TE_randRange(16, 112);
            //             enemies[i].character.targetY = TE_randRange(16, 112);
            //             float tdx = enemies[i].character.targetX - enemies[i].character.x;
            //             float tdy = enemies[i].character.targetY - enemies[i].character.y;
            //             float tsqd = tdx*tdx+tdy*tdy;
            //             if (tsqd < 12.0f) continue;
            //             float tx = enemies[i].character.targetX;
            //             float ty = enemies[i].character.targetY;
            //             Character_toBaseF(&enemies[i].character, &tx, &ty);
            //             int16_t nearestX, nearestY;
            //             float sdf = Environment_calcSDFValue(tx, ty, &nearestX, &nearestY);
            //             if (sdf < 12.0f) continue;
            //             break;
            //         }
            //     }
            // }
            // else
            // {
            //     enemies[i].idleTime = 0.0f;
            // }
        }
    }
}

int Enemies_raycastPoint(float x, float y)
{
    float rad2 = ENEMY_RADIUS * ENEMY_RADIUS;
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f)
        {
            float dx = x - enemies[i].character.x;
            float dy = y - enemies[i].character.y;
            if (dx >= ENEMY_RECT_X && dx < ENEMY_RECT_X + ENEMY_RECT_WIDTH && dy >= ENEMY_RECT_Y && dy < ENEMY_RECT_Y + ENEMY_RECT_HEIGHT)
            {
                return i;
            }
        }
    }
    return -1;
}

int Enemies_getPosition(uint8_t id, float *outX, float *outY)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f && enemies[i].id == id)
        {
            *outX = enemies[i].character.x;
            *outY = enemies[i].character.y;
            return 1;
        }
    }
    return 0;
}

void Enemies_setHealth(uint8_t id, float health)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f && enemies[i].id == id)
        {
            enemies[i].health = health;
            break;
        }
    }
}

int Enemies_isAlive(uint8_t id)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f && enemies[i].id == id)
        {
            return 1;
        }
    }
    return 0;
}