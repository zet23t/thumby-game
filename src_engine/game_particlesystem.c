#include "game_particlesystem.h"

#include "game.h"
#include "game_assets.h"
#include "TE_Image.h"
#include "TE_rand.h"

#define MAX_PARTICLES 64

typedef struct Particle
{
    uint8_t type;
    uint8_t z;
    float life;
    float x, y;
    float vx, vy;
    ParticleTypeData typeData;
} Particle;

typedef struct ParticleSystem
{
    Particle particles[MAX_PARTICLES];
    uint8_t count;
} ParticleSystem;

static ParticleSystem _system;

void ParticleSystem_init()
{
    _system.count = 0;
}

void ParticleSystem_update(RuntimeContext *ctx, TE_Img *screen)
{
    uint32_t oldSeed = TE_randSetSeed(ctx->frameCount * 7 + 392);
    float dt = ctx->deltaTime;
    int writeIndex = 0;
    for (int i = 0; i < _system.count; i++)
    {
        Particle p = _system.particles[i];
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life += dt;
        uint8_t despawn = 0;
        switch (p.type)
        {
        case PARTICLE_TYPE_SIMPLE:
            if (p.life > p.typeData.simpleType.maxLife)
            {
                despawn = 1;
            }
            else
            {
                p.vx += p.typeData.simpleType.accelX * dt;
                p.vy += p.typeData.simpleType.accelY * dt;
                p.vx *= 1.0f - p.typeData.simpleType.drag * dt;
                p.vy *= 1.0f - p.typeData.simpleType.drag * dt;
                uint8_t size = p.typeData.simpleType.size + 1;
                TE_Img_fillRect(screen, p.x - (size >> 1), p.y - (size >> 1), size, size, p.typeData.simpleType.color, (TE_ImgOpState){
                    .zValue = p.z,
                });
                
            }
            break;
        default:
            despawn = 1;
            break;
        }
        if (!despawn)
        {
            _system.particles[writeIndex++] = p;
        }
    }
    _system.count = writeIndex;

    TE_randSetSeed(oldSeed);
}

void ParticleSystem_spawn(uint8_t type, float x, float y, uint8_t z, float vx, float vy, ParticleTypeData typeData)
{
    if (_system.count >= MAX_PARTICLES)
        return;
    Particle *p = &_system.particles[_system.count++];
    p->type = type;
    p->life = 0;
    p->x = x;
    p->y = y;
    p->z = z;
    p->vx = vx;
    p->vy = vy;
    p->typeData = typeData;
}