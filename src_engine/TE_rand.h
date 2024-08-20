#ifndef TE_RAND_H
#define TE_RAND_H

#include <inttypes.h>

uint32_t TE_rand();
int32_t TE_randRange(int32_t min, int32_t max);
void TE_randSetSeed(uint32_t seed);


#endif // TE_RAND_H