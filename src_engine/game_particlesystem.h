#ifndef __GAME_PARTICLESYSTEM_H__
#define __GAME_PARTICLESYSTEM_H__

#include "game.h"
#include "TE_Image.h"
#include <inttypes.h>

typedef union ParticleTypeData
{
    struct
    {
        uint32_t color;
        uint8_t size:3;
        float maxLife;
        float accelX, accelY;
        float drag;
    } simpleType;
} ParticleTypeData;

#define PARTICLE_TYPE_SIMPLE 0

void ParticleSystem_init();
void ParticleSystem_update(RuntimeContext *ctx, TE_Img *screen);
void ParticleSystem_spawn(uint8_t type, float x, float y, uint8_t z, float vx, float vy, ParticleTypeData typeData);

#endif