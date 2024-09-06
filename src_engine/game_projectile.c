#include "game.h"

#include <math.h>
#include "game_enemies.h"
#include "TE_rand.h"
#include <stdio.h>
#include "game_environment.h"

void Projectile_spawn(float x, float y, float vx, float vy, uint32_t color)
{
    for (int i=0;i<PROJECTILE_MAX_COUNT;i++)
    {
        if (projectiles[i].lifeTime <= 0.0f)
        {
            float len = sqrtf(vx * vx + vy * vy);
            projectiles[i] = (Projectile) {
                .lifeTime = 2.0f,
                .x = x,
                .y = y,
                .vx = vx,
                .vy = vy,
                .nx = vx / len,
                .ny = vy / len,
                .color = color,
            };
            
            break;
        }
    }
}

void Projectiles_update(Projectile *projectile, RuntimeContext *ctx, TE_Img *img)
{
    for (int i=0;i<PROJECTILE_MAX_COUNT;i++)
    {
        if (projectiles[i].lifeTime > 0.0f)
        {
            projectiles[i].lifeTime -= ctx->deltaTime;
            projectiles[i].x += projectiles[i].vx * ctx->deltaTime;
            projectiles[i].y += projectiles[i].vy * ctx->deltaTime;

            int hit = Enemies_raycastPoint(projectiles[i].x, projectiles[i].y);
            if (hit >= 0)
            {
                enemies[hit].health -= 1.0f;
                projectiles[i].lifeTime = 0.0f;

                Enemy_takeDamage(&enemies[hit], 1.0f, projectiles[i].vx, projectiles[i].vy, ctx, img);
                continue;
            }

            hit = Environment_raycastPoint(projectiles[i].x, projectiles[i].y);
            if (hit >= 0)
            {
                projectiles[i].lifeTime = 0.0f;
                printf("Hit environment %d\n", hit);
                continue;
            }

            if (projectiles[i].x < 0 || projectiles[i].x >= 128 || projectiles[i].y < 0 || projectiles[i].y >= 128)
            {
                projectiles[i].lifeTime = 0.0f;
                continue;
            }
            
            int16_t x = (int16_t) floorf(projectiles[i].x);
            int16_t y = (int16_t) floorf(projectiles[i].y);
            int16_t x2 = (int16_t) floorf(projectiles[i].x - projectiles[i].nx * 6.0f);
            int16_t y2 = (int16_t) floorf(projectiles[i].y - projectiles[i].ny * 6.0f);
            int16_t lx = (int16_t) floorf(projectiles[i].x - projectiles[i].nx * 2.0f + projectiles[i].ny);
            int16_t ly = (int16_t) floorf(projectiles[i].y - projectiles[i].ny * 2.0f - projectiles[i].nx);
            int16_t rx = (int16_t) floorf(projectiles[i].x - projectiles[i].nx * 2.0f - projectiles[i].ny);
            int16_t ry = (int16_t) floorf(projectiles[i].y - projectiles[i].ny * 2.0f + projectiles[i].nx);
            TE_ImgOpState state = {
                .zCompareMode = Z_COMPARE_ALWAYS,
                .zValue = (uint8_t) projectiles[i].y,
            };
            TE_Img_line(img, x,y, x2, y2, projectiles[i].color, state);
            TE_Img_line(img, x,y, lx, ly, projectiles[i].color, state);
            TE_Img_line(img, x,y, rx, ry, projectiles[i].color, state);
        
        }
    }
}