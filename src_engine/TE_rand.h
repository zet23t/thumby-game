#ifndef TE_RAND_H
#define TE_RAND_H

#include <inttypes.h>

uint32_t TE_rand();
int32_t TE_randRange(int32_t min, int32_t max);
void TE_randRadius(int16_t radius, int32_t *x, int32_t *y);
uint32_t TE_randSetSeed(uint32_t seed);
void TE_addEntropy(uint32_t entropy);
uint32_t TE_randGetSeed();

#endif // TE_RAND_H