#ifndef __TE_DEBUG_H__
#define __TE_DEBUG_H__

#include <inttypes.h>

void TE_Debug_drawPixel(int x, int y, uint32_t color);
void TE_Debug_drawLine(int x1, int y1, int x2, int y2, uint32_t color);
void TE_Debug_drawLineCircle(int x, int y, int r, uint32_t color);
void TE_Debug_drawText(int x, int y, const char *text, uint32_t color);

#endif // __TE_DEBUG_H__