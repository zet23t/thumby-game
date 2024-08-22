#ifndef TE_RAND_H
#define TE_RAND_H

#include <inttypes.h>

uint32_t TE_rand();
int32_t TE_randRange(int32_t min, int32_t max);
void TE_randRadius(int16_t radius, int32_t *x, int32_t *y);
void TE_randSetSeed(uint32_t seed);

#endif // TE_RAND_H