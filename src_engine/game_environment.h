#ifndef __GAME_ENVIRONMENT_H__
#define __GAME_ENVIRONMENT_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"

void Environment_init();
void Environment_addTree(int16_t x, int16_t y, uint32_t seed);
void Environment_addTreeGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius);
void Environment_update(RuntimeContext *ctx, TE_Img* img);

#endif // __GAME_ENVIRONMENT_H__