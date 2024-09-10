#include "TE_sdfmap.h"
#include "TE_debug.h"
#include <math.h>


TE_SDFCell TE_SDFMap_getCell(TE_SDFMap *map, int16_t x, int16_t y)
{
    if (x < 0 || x >= map->width || y < 0 || y >= map->height)
    {
        TE_SDFCell cell = {0};
        return cell;
    }
    return map->data[y * map->width + x];
}

void TE_SDFMap_setSolid(TE_SDFMap *map, int16_t x, int16_t y, uint8_t solid)
{
    if (x < 0 || x >= map->width || y < 0 || y >= map->height)
    {
        return;
    }
    map->data[y * map->width + x].solid = solid;
}

void TE_SDFMap_drawDebug(TE_SDFMap *map, RuntimeContext *ctx)
{
    static int16_t cursorX = 0, cursorY = 0;
    if (ctx->inputUp) cursorY--;
    if (ctx->inputDown) cursorY++;
    if (ctx->inputLeft) cursorX--;
    if (ctx->inputRight) cursorX++;
    if (cursorX < 0) cursorX = map->width - 1;
    if (cursorY < 0) cursorY = map->height - 1;
    if (cursorX >= map->width) cursorX = 0;
    if (cursorY >= map->height) cursorY = 0;

    uint16_t maxSqDist = 0;
    uint32_t pixels = map->width * map->height;
    for (int i=0;i<pixels;i++)
    {
        if (map->data[i].sqDistance > maxSqDist)
        {
            maxSqDist = map->data[i].sqDistance;
        }
    }
    float maxDist = sqrt(maxSqDist);

    TE_SDFCell *cell = map->data;
    for (int y=0;y<map->height;y++)
    {
        for (int x=0;x<map->width;x++, cell++)
        {
            if (!cell->passed) {
                TE_Debug_drawPixel(x, y, 0xffff00ff);
                continue;
            }
            if (cell->sqDistance == 0) {
                TE_Debug_drawPixel(x, y, 0xff0000ff);
                continue;
            }
            uint8_t distance = sqrt(cell->sqDistance) * 255 / maxDist;
            uint32_t color = distance << (cell->solid ? 8 : 16);
            TE_Debug_drawPixel(x, y, color | 0xff000000);
        }
    }
    TE_Debug_drawPixel(cursorX, cursorY, 0xffffffff);
    TE_SDFCell cursorCell = TE_SDFMap_getCell(map, cursorX, cursorY);
    if (cursorCell.passed)
    {
        TE_Debug_drawLine(cursorX, cursorY, cursorX + cursorCell.dx, cursorY + cursorCell.dy, 0xff00ff00);
    }
    TE_Debug_drawText(2, 2, TE_StrFmt("(%d %d)", cursorX, cursorY), 0xffffffff);
}

void TE_SDFMap_compute(TE_SDFMap *map)
{
    for (int i=0;i<map->width*map->height;i++)
    {
        map->data[i].passed = 0;
    }
    int8_t dxMap[] = {-1, 1, 0, 0, -1, 1, -1, 1};
    int8_t dyMap[] = {0, 0, -1, 1, -1, -1, 1, 1};

    uint32_t passCount = map->width * map->height;
    for (int pass=0;pass < 32 && passCount > 0;pass++)
    {
        TE_SDFCell *cell = map->data;
        uint32_t distCap = pass * pass + 1;
        for (int y = 0; y < map->height; y++)
        {
            for (int x = 0; x < map->width; x++, cell++)
            {
                if (cell->passed)
                {
                    continue;
                }
                TE_SDFCell neighbors[] = {
                    TE_SDFMap_getCell(map, x-1, y),
                    TE_SDFMap_getCell(map, x+1, y),
                    TE_SDFMap_getCell(map, x, y-1),
                    TE_SDFMap_getCell(map, x, y+1),
                    TE_SDFMap_getCell(map, x-1, y-1),
                    TE_SDFMap_getCell(map, x+1, y-1),
                    TE_SDFMap_getCell(map, x-1, y+1),
                    TE_SDFMap_getCell(map, x+1, y+1),
                };
                uint8_t solid4Count = cell->solid;
                for (int i=0;i<4;i++) solid4Count += neighbors[i].solid;
                // case: it's solid and a wall
                if (cell->solid && solid4Count < 5)
                {
                    cell->sqDistance = 0;
                    cell->dx = cell->dy = 0;
                    cell->passed = 1;
                    passCount--;
                    continue;
                }
                // case: it's not solid and a wall
                if (!cell->solid && solid4Count > 0)
                {
                    cell->sqDistance = 1;
                    cell->passed = 1;
                    passCount--;
                    if (neighbors[0].solid) cell->dx = -1, cell->dy = 0;
                    else if (neighbors[1].solid) cell->dx = 1, cell->dy = 0;
                    else if (neighbors[2].solid) cell->dx = 0, cell->dy = -1;
                    else if (neighbors[3].solid) cell->dx = 0, cell->dy = 1;
                    continue;
                }

                // find the minimum distance of a passed neighbor
                TE_SDFCell *minCell = 0;
                uint8_t minCellIdx = 0;
                for (int i=0;i<8;i++)
                {
                    if (neighbors[i].passed)
                    {
                        if (!minCell || neighbors[i].sqDistance < minCell->sqDistance)
                        {
                            minCell = &neighbors[i];
                            minCellIdx = i;
                        }
                    }
                }
                if (!minCell || minCell->sqDistance >= distCap)
                {
                    continue;
                }
                int8_t dx = dxMap[minCellIdx] + minCell->dx;
                int8_t dy = dyMap[minCellIdx] + minCell->dy;
                int16_t sqDist = dx*dx + dy*dy;
                cell->sqDistance = sqDist;
                cell->dx = dx;
                cell->dy = dy;
                cell->passed = 1;
                passCount--;
            }
        }
    }
}

void TE_SDFMap_addCircle(TE_SDFMap *map, int16_t x, int16_t y, int16_t radius)
{
    for (int dy=-radius;dy<=radius;dy++)
    {
        for (int dx=-radius;dx<=radius;dx++)
        {
            if (dx*dx + dy*dy <= radius*radius)
            {
                TE_SDFMap_setSolid(map, x+dx, y+dy, 1);
            }
        }
    }
}

void TE_SDFMap_setRect(TE_SDFMap *map, int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t solid)
{
    for (int dy=0;dy<height;dy++)
    {
        for (int dx=0;dx<width;dx++)
        {
            TE_SDFMap_setSolid(map, x+dx, y+dy, solid);
        }
    }
}