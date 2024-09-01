#ifndef __GAME_ENVIRONMENT_H__
#define __GAME_ENVIRONMENT_H__

#include <inttypes.h>
#include "engine_main.h"
#include "TE_Image.h"

void Environment_init();
void Environment_addBushGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius);
void Environment_addTree(int16_t x, int16_t y, uint32_t seed);
void Environment_addTreeGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius);
void Environment_addFlowerGroup(int16_t x, int16_t y, uint32_t seed, uint8_t count, uint8_t scatterRadius);
int Environment_raycastPoint(int16_t px, int16_t py);
int Environment_raycastCircle(int16_t px, int16_t py, int16_t radius, int16_t *outCenterX, int16_t *outCenterY, int16_t *outRadius);
void Environment_update(RuntimeContext *ctx, TE_Img* img);
float Environment_calcSDFValue(int16_t px, int16_t py, int16_t *nearestX, int16_t *nearestY);
#endif // __GAME_ENVIRONMENT_H__