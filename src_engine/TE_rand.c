#include "TE_rand.h"

static int TE_seed = 0x3291;

uint32_t TE_randGetSeed()
{
    return TE_seed;
}

uint32_t TE_randSetSeed(uint32_t seed)
{
    uint32_t oldSeed = TE_seed;
    TE_seed = seed;
    return oldSeed;
}

void TE_addEntropy(uint32_t entropy)
{
    TE_seed ^= entropy;
}

uint32_t TE_rand()
{
    TE_seed = TE_seed * 1103515245 + 12345;
    return (TE_seed / 65536) % 32768;
}

int32_t TE_randRange(int32_t min, int32_t max)
{
    return min + TE_rand() % (max - min);
}

void TE_randRadius(int16_t radius, int32_t *x, int32_t *y)
{
    int r2 = radius * radius;
    while (1)
    {
        int32_t dx = TE_randRange(-radius, radius + 1);
        int32_t dy = TE_randRange(-radius, radius + 1);
        if (dx * dx + dy * dy <= r2)
        {
            *x = dx;
            *y = dy;
            return;
        }
    }
}