#ifndef __TE_SDFMAP_H__
#define __TE_SDFMAP_H__

#include <inttypes.h>
#include "game.h"

typedef struct TE_SDFCell
{
    uint8_t solid:1;
    uint8_t passed:1;
    int8_t dx:7;
    int8_t dy:7;
    uint16_t sqDistance;
} TE_SDFCell;

typedef struct TE_SDFMap
{
    uint16_t width;
    uint16_t height;
    TE_SDFCell *data;
} TE_SDFMap;

TE_SDFCell TE_SDFMap_getCell(TE_SDFMap *map, int16_t x, int16_t y);
void TE_SDFMap_setSolid(TE_SDFMap *map, int16_t x, int16_t y, uint8_t solid);
void TE_SDFMap_compute(TE_SDFMap *map);
void TE_SDFMap_drawDebug(TE_SDFMap *map, RuntimeContext *ctx);
void TE_SDFMap_addCircle(TE_SDFMap *map, int16_t x, int16_t y, int16_t radius);
void TE_SDFMap_setRect(TE_SDFMap *map, int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t solid);
#endif // __TE_SDFMAP_H__