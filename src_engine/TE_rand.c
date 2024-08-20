#include "TE_rand.h"

static int TE_seed = 0x3291;

void TE_randSetSeed(uint32_t seed)
{
    TE_seed = seed;
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