#include "game_enemies.h"
#include "game_character.h"
#include "TE_rand.h"

int Enemies_spawn(int type, int x, int y)
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
        .character = enemyCharacters[type],
    };
    enemies[index].character.x = x;
    enemies[index].character.y = y;
    enemies[index].character.dirY = 1;
    enemies[index].character.targetX = x;
    enemies[index].character.targetY = y;
    return index;
}

void Enemies_update(RuntimeContext *ctx, TE_Img *img)
{
    for (int i=0;i<MAX_ENEMIES;i++)
    {
        if (enemies[i].health > 0.0f)
        {
            Character_update(&enemies[i].character, ctx, img, enemies[i].character.targetX, enemies[i].character.targetY, enemies[i].character.dirX, enemies[i].character.dirY);
            if (enemies[i].character.targetDistance < 2.0f)
            {
                enemies[i].idleTime += ctx->deltaTime;
                if (enemies[i].idleTime > 2.0f)
                {
                    uint32_t seed = (uint32_t) (enemies[i].character.x * 100.0f) + (uint32_t) (enemies[i].character.y * 1000.0f);
                    TE_randSetSeed(seed);
                    enemies[i].character.targetX = TE_randRange(16, 112);
                    enemies[i].character.targetY = TE_randRange(16, 112);
                }
            }
            else
            {
                enemies[i].idleTime = 0.0f;
            }
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
            float dx = enemies[i].character.x - x;
            float dy = enemies[i].character.y - y;
            if (dx * dx + dy * dy < rad2)
            {
                return i;
            }
        }
    }
    return -1;
}