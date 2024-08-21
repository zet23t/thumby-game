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
            // TE_Img_lineRect(img, enemies[i].character.x + ENEMY_RECT_X, enemies[i].character.y + ENEMY_RECT_Y, ENEMY_RECT_WIDTH, ENEMY_RECT_HEIGHT, 0xff00FF00, (TE_ImgOpState){0});
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