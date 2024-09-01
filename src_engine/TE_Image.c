#include "TE_Image.h"

void TE_Img_setPixel(TE_Img *img, uint16_t x, uint16_t y, uint32_t color, TE_ImgOpState state)
{
    if (x >= (1 << img->p2width) || y >= (1 << img->p2height))
    {
        return;
    }
    if (state.scissorWidth > 0 || state.scissorHeight > 0)
    {
        if (x < state.scissorX || x >= state.scissorX + state.scissorWidth || y < state.scissorY || y >= state.scissorY + state.scissorHeight)
        {
            return;
        }
    }

    uint32_t *pixel = &img->data[(y << img->p2width) + x];
    uint8_t zDst = *pixel >> 24;
    if ((state.zCompareMode == Z_COMPARE_ALWAYS) ||
        (state.zCompareMode == Z_COMPARE_EQUAL && zDst == state.zValue) ||
        (state.zCompareMode == Z_COMPARE_LESS && zDst < state.zValue) ||
        (state.zCompareMode == Z_COMPARE_GREATER && zDst > state.zValue) ||
        (state.zCompareMode == Z_COMPARE_LESS_EQUAL && zDst <= state.zValue) ||
        (state.zCompareMode == Z_COMPARE_GREATER_EQUAL && zDst >= state.zValue) ||
        (state.zCompareMode == Z_COMPARE_NOT_EQUAL && zDst != state.zValue)
    )
    {
        if (state.zNoWrite)
        {
            color = (color & 0xffffff) | (zDst << 24);
        }
        else
        {
            color = (color & 0xffffff) | (state.zValue << 24);
        }
        *pixel = color;
    }
}


uint32_t TE_Img_getPixelEx(TE_Img *img, uint16_t ox, uint16_t oy, uint16_t x, uint16_t y, uint16_t w, uint16_t h, BlitEx options)
{
    if (options.flipX)
    {
        x = w - x - 1;
    }
    if (options.flipY)
    {
        y = h - y - 1;
    }
    if (options.rotate == 1)
    {
        uint16_t tmp = x;
        x = y;
        y = w - tmp - 1;
    }
    else if (options.rotate == 2)
    {
        x = w - x - 1;
        y = h - y - 1;
    }
    else if (options.rotate == 3)
    {
        uint16_t tmp = x;
        x = h - y - 1;
        y = tmp;
    }

    x += ox;
    y += oy;

    if (x >= (1 << img->p2width) || y >= (1 << img->p2height))
    {
        return 0;
    }

    return img->data[(y << img->p2width) + x];
}

void TE_Img_blitSprite(TE_Img *img, TE_Sprite sprite, int16_t x, int16_t y, BlitEx options)
{
    TE_Img_blitEx(img, sprite.img, x - sprite.pivotX, y - sprite.pivotY, sprite.src.x, sprite.src.y, sprite.src.width, sprite.src.height, options);
}


static int absi(int x)
{
    return x < 0 ? -x : x;
}

void TE_Img_lineTriangle(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color, TE_ImgOpState state)
{
    if (y0 > y1)
    {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y1 > y2)
    {
        int16_t tmp = y1;
        y1 = y2;
        y2 = tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y0 > y1)
    {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    
    int16_t dx01 = x1 - x0;
    int16_t dy01 = y1 - y0;
    int16_t dx02 = x2 - x0;
    int16_t dy02 = y2 - y0;
    int16_t dx12 = x2 - x1;
    int16_t dy12 = y2 - y1;

    int16_t x, y;
    if (dy01 == 0)
    {
        dy01 = 1;
    }
    if (dy02 == 0)
    {
        dy02 = 1;
    }
    if (dy12 == 0)
    {
        dy12 = 1;
    }
    for (y = y0; y <= y1; y++)
    {
        int16_t xL = x0 + dx01 * (y - y0) / dy01;
        int16_t xR = x0 + dx02 * (y - y0) / dy02;
        if (xL > xR)
        {
            int16_t tmp = xL;
            xL = xR;
            xR = tmp;
        }
        
        // todo: fill in case angle is < 45 degrees
        TE_Img_setPixel(img, xL, y, color, state);
        TE_Img_setPixel(img, xR, y, color, state);
        
    }
    for (y = y1; y <= y2; y++)
    {
        int16_t xL = x1 + dx12 * (y - y1) / dy12;
        int16_t xR = x0 + dx02 * (y - y0) / dy02;
        if (xL > xR)
        {
            int16_t tmp = xL;
            xL = xR;
            xR = tmp;
        }
        // todo: fill in case angle is < 45 degrees
        TE_Img_setPixel(img, xL, y, color, state);
        TE_Img_setPixel(img, xR, y, color, state);
    }
}

void TE_Img_fillTriangle(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color, TE_ImgOpState state)
{
    if (y0 > y1)
    {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y1 > y2)
    {
        int16_t tmp = y1;
        y1 = y2;
        y2 = tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y0 > y1)
    {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    
    int16_t dx01 = x1 - x0;
    int16_t dy01 = y1 - y0;
    int16_t dx02 = x2 - x0;
    int16_t dy02 = y2 - y0;
    int16_t dx12 = x2 - x1;
    int16_t dy12 = y2 - y1;

    int16_t x, y;
    if (dy01 == 0)
    {
        dy01 = 1;
    }
    if (dy02 == 0)
    {
        dy02 = 1;
    }
    if (dy12 == 0)
    {
        dy12 = 1;
    }
    for (y = y0; y <= y1; y++)
    {
        int16_t xL = x0 + dx01 * (y - y0) / dy01;
        int16_t xR = x0 + dx02 * (y - y0) / dy02;
        if (xL > xR)
        {
            int16_t tmp = xL;
            xL = xR;
            xR = tmp;
        }
        for (x = xL; x <= xR; x++)
        {
            TE_Img_setPixel(img, x, y, color, state);
        }
    }
    for (y = y1; y <= y2; y++)
    {
        int16_t xL = x1 + dx12 * (y - y1) / dy12;
        int16_t xR = x0 + dx02 * (y - y0) / dy02;
        if (xL > xR)
        {
            int16_t tmp = xL;
            xL = xR;
            xR = tmp;
        }
        for (x = xL; x <= xR; x++)
        {
            TE_Img_setPixel(img, x, y, color, state);
        }
    }
}

int lineRectClip(int16_t rectX, int16_t rectY, int16_t rectW, int16_t rectH, int16_t *x1, int16_t *y1, int16_t *x2, int16_t *y2)
{
    if (*x1 < rectX)
    {
        if (*x2 == *x1)
        {
            return 0;
        }
        *y1 += (*y2 - *y1) * (rectX - *x1) / (*x2 - *x1);
        *x1 = rectX;
    }
    if (*x1 >= rectX + rectW)
    {
        if (*x2 == *x1)
        {
            return 0;
        }
        *y1 += (*y2 - *y1) * (rectX + rectW - *x1) / (*x2 - *x1);
        *x1 = rectX + rectW;
    }

    if (*x2 < rectX)
    {
        *y2 += (*y2 - *y1) * (rectX - *x2) / (*x2 - *x1);
        *x2 = rectX;
    }
    if (*x2 >= rectX + rectW)
    {
        *y2 += (*y2 - *y1) * (rectX + rectW - *x1) / (*x2 - *x1);
        *x2 = rectX + rectW;
    }

    if (*y1 < rectY)
    {
        if (*y2 == *y1)
        {
            return 0;
        }
        *x1 += (*x2 - *x1) * (rectY - *y1) / (*y2 - *y1);
        *y1 = rectY;
    }

    if (*y1 >= rectY + rectH)
    {
        if (*y2 == *y1)
        {
            return 0;
        }
        *x1 += (*x2 - *x1) * (rectY + rectH - *y1) / (*y2 - *y1);
        *y1 = rectY + rectH;
    }

    if (*y2 < rectY)
    {
        *x2 += (*x2 - *x1) * (rectY - *y2) / (*y2 - *y1);
        *y2 = rectY;
    }

    if (*y2 >= rectY + rectH)
    {
        *x2 += (*x2 - *x1) * (rectY + rectH - *y1) / (*y2 - *y1);
        *y2 = rectY + rectH;
    }

    return (*x1 > rectX && *x1 < rectX + rectW && *y1 > rectY && *y1 < rectY + rectH)
        || (*x2 > rectX && *x2 < rectX + rectW && *y2 > rectY && *y2 < rectY + rectH);
}

void TE_Img_line(TE_Img *img, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, TE_ImgOpState state)
{
    if (x0 == x1 && y0 == y1)
    {
        TE_Img_setPixel(img, x0, y0, color, state);
        return;
    }

    if (!lineRectClip(0, 0, 1 << img->p2width, 1 << img->p2height, &x0, &y0, &x1, &y1))
    {
        return;
    }

    int16_t dx = absi(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = -absi(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (1)
    {
        TE_Img_setPixel(img, x0, y0, color, state);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        int16_t e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

uint32_t TE_Color_tint(uint32_t color, uint32_t tint)
{
    uint32_t r = ((color & 0xFF) * (tint & 0xFF)) >> 8;
    uint32_t g = (((color >> 8) & 0xFF) * ((tint >> 8) & 0xFF)) >> 8;
    uint32_t b = (((color >> 16) & 0xFF) * ((tint >> 16) & 0xFF)) >> 8;
    uint32_t a = (((color >> 24) & 0xFF) * ((tint >> 24) & 0xFF)) >> 8;
    return r | (g << 8) | (b << 16) | (a << 24);
}

void TE_Img_blitEx(TE_Img *img, TE_Img *src, int16_t x, int16_t y, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height, BlitEx options)
{
    for (uint16_t i = 0; i < width; i++)
    {
        for (uint16_t j = 0; j < height; j++)
        {
            uint32_t color = TE_Img_getPixelEx(src, srcX, srcY, i, j, width, height, options);
            if (options.tint)
            {
                color = TE_Color_tint(color, options.tintColor);
            }

            if (options.blendMode == TE_BLEND_ALPHAMASK)
            {
                if ((color & 0xFF000000) == 0)
                {
                    continue;
                }
            }

            TE_Img_setPixel(img, x + i, y + j, color, options.state);
        }
    }
}

void TE_Img_clear(TE_Img *img, uint32_t color, uint8_t z)
{
    color = (color & 0xffffff) | (z << 24);
    for (uint32_t i = 0; i < (1 << img->p2width) * (1 << img->p2height); i++)
    {
        img->data[i] = color;
    }
}

void TE_Img_HLine(TE_Img *img, int16_t x, int16_t y, uint16_t w, uint32_t color, TE_ImgOpState state)
{
    if (x >= (1 << img->p2width) || y >= (1 << img->p2height) || y < 0)
    {
        return;
    }
    int16_t x2 = x + w;
    if (x2 < 0)
    {
        return;
    }
    x = x < 0 ? 0 : x;
    x2 = x2 >= (1 << img->p2width) ? (1 << img->p2width) - 1 : x2;

    for (uint16_t i = x; i < x2; i++)
    {
        TE_Img_setPixel(img, i, y, color, state);
    }
}

void TE_Img_VLine(TE_Img *img, int16_t x, int16_t y, uint16_t h, uint32_t color, TE_ImgOpState state)
{
    if (x >= (1 << img->p2width) || y >= (1 << img->p2height) || x < 0)
    {
        return;
    }
    int16_t y2 = y + h;
    if (y2 < 0)
    {
        return;
    }
    y = y < 0 ? 0 : y;
    y2 = y2 >= (1 << img->p2height) ? (1 << img->p2height) - 1 : y2;

    for (uint16_t i = y; i < y2; i++)
    {
        TE_Img_setPixel(img, x, i, color, state);
    }
}

void TE_Img_lineRect(TE_Img *img, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t color, TE_ImgOpState state)
{
    TE_Img_HLine(img, x, y, w, color, state);
    TE_Img_HLine(img, x, y + h - 1, w, color, state);
    TE_Img_VLine(img, x, y, h, color, state);
    TE_Img_VLine(img, x + w - 1, y, h, color, state);
}

void TE_Img_fillRect(TE_Img *img, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t color, TE_ImgOpState state)
{
    int16_t x1 = x < 0 ? 0 : x;
    int16_t y1 = y < 0 ? 0 : y;
    int16_t x2 = x + w;
    int16_t y2 = y + h;

    if (x1 >= (1 << img->p2width) || y1 >= (1 << img->p2height) || x2 < 0 || y2 < 0)
    {
        return;
    }


    x2 = x2 >= (1 << img->p2width) ? (1 << img->p2width) : x2;
    y2 = y2 >= (1 << img->p2height) ? (1 << img->p2height) : y2;

    for (uint16_t i = y1; i < y2; i++)
    {
        for (uint16_t j = x1; j < x2; j++)
        {
            TE_Img_setPixel(img, j, i, color, state);
        }
    }
}

void TE_Img_fillCircle(TE_Img *img, int16_t x, int16_t y, uint16_t radius, uint32_t color, TE_ImgOpState state)
{
    int16_t x0 = radius;
    int16_t y0 = 0;
    int16_t err = radius < 5 ? 4 - 2 * radius : -5;

    while (x0 >= y0)
    {
        TE_Img_HLine(img, x - x0, y + y0, x0 * 2, color, state);
        TE_Img_HLine(img, x - x0, y - y0, x0 * 2, color, state);
        TE_Img_HLine(img, x - y0, y + x0, y0 * 2, color, state);
        TE_Img_HLine(img, x - y0, y - x0, y0 * 2, color, state);

        if (err <= 0)
        {
            y0 += 1;
            err += 2 * y0 + 1;
        }
        if (err > 0)
        {
            x0 -= 1;
            err -= 2 * x0 + 1;
        }
    }
}
void TE_Img_lineCircle(TE_Img *img, int16_t x, int16_t y, uint16_t radius, uint32_t color, TE_ImgOpState state)
{
    int16_t x0 = radius;
    int16_t y0 = 0;
    int16_t err = radius < 5 ? 4 - 2 * radius : -5;

    while (x0 >= y0)
    {
        TE_Img_setPixel(img, x + x0, y + y0, color, state);
        TE_Img_setPixel(img, x + y0, y + x0, color, state);
        TE_Img_setPixel(img, x - y0, y + x0, color, state);
        TE_Img_setPixel(img, x - x0, y + y0, color, state);
        TE_Img_setPixel(img, x - x0, y - y0, color, state);
        TE_Img_setPixel(img, x - y0, y - x0, color, state);
        TE_Img_setPixel(img, x + y0, y - x0, color, state);
        TE_Img_setPixel(img, x + x0, y - y0, color, state);

        if (err <= 0)
        {
            y0 += 1;
            err += 2 * y0 + 1;
        }
        if (err > 0)
        {
            x0 -= 1;
            err -= 2 * x0 + 1;
        }
    }
}

void TE_Img_drawPatch9(TE_Img *img, TE_Img* src, int16_t x, int16_t y, int16_t w, int16_t h,
    int16_t srcX, int16_t srcY, uint8_t cellWidth, uint8_t cellHeight, BlitEx options)
{
    if (w <= 0 || h <= 0 || x > (1 << img->p2width) || y > (1 << img->p2height))
    {
        return;
    }
    int16_t d3x = w / 2;
    int16_t d3y = h / 2;

    int16_t cornerW1 = d3x < cellWidth ? d3x : cellWidth;
    int16_t cornerH1 = d3y < cellHeight ? d3y  : cellHeight;
    int16_t cornerW2 = cornerW1;
    int16_t cornerH2 = cornerH1;

    if (cornerW1 < cellWidth) cornerW1 = w - cornerW1;
    if (cornerH1 < cellHeight) cornerH1 = h - cornerH1;

    int16_t tlx = x;
    int16_t tly = y;

    int16_t trx = x + w - cornerW2;
    int16_t try = y;

    int16_t blx = x;
    int16_t bly = y + h - cornerH2;

    int16_t brx = x + w - cornerW2;
    int16_t bry = y + h - cornerH2;

    // TE_Img_lineRect(img, x, y, w, h, 0xffffffff, options.state);
    // TE_Img_lineRect(img, tlx, tly, cornerW1, cornerH1, 0xff00ffff, options.state);
    // TE_Img_lineRect(img, trx, try, cornerW2, cornerH1, 0xff00ffff, options.state);
    // TE_Img_lineRect(img, blx, bly, cornerW1, cornerH2, 0xff00ffff, options.state);
    // TE_Img_lineRect(img, brx, bry, cornerW2, cornerH2, 0xff00ffff, options.state);

    int16_t px, py;
    TE_Img_blitEx(img, src, tlx, tly, srcX, srcY, cornerW1, cornerH1, options);
    TE_Img_blitEx(img, src, trx, try, srcX + cellWidth * 3 - cornerW2, srcY, cornerW2, cornerH1, options);
    for (px = tlx + cornerW1; px <= trx - cellWidth; px+= cellWidth)
    {
        TE_Img_blitEx(img, src, px, tly, srcX + cellWidth, srcY, cellWidth, cornerH1, options);
    }
    if (px < trx)
    {
        TE_Img_blitEx(img, src, px, tly, srcX + cellWidth, srcY, trx - px, cornerH1, options);
    }

    TE_Img_blitEx(img, src, blx, bly, srcX, srcY + cellHeight * 3 - cornerH2, cornerW1, cornerH2, options);
    TE_Img_blitEx(img, src, brx, bry, srcX + cellWidth * 3 - cornerW2, srcY + cellHeight * 3 - cornerH2, cornerW2, cornerH2, options);
    for (px = blx + cornerW1; px <= brx - cellWidth; px+= cellWidth)
    {
        TE_Img_blitEx(img, src, px, bly, srcX + cellWidth, srcY + cellHeight * 3 - cornerH2, cellWidth, cornerH2, options);
    }
    if (px < brx)
    {
        TE_Img_blitEx(img, src, px, bly, srcX + cellWidth, srcY + cellHeight * 3 - cornerH2, brx - px, cornerH2, options);
    }

    for (py = tly + cornerH1; py <= bly - cellHeight; py+= cellHeight)
    {
        TE_Img_blitEx(img, src, tlx, py, srcX, srcY + cellHeight, cornerW1, cellHeight, options);
        TE_Img_blitEx(img, src, trx, py, srcX + cellWidth * 3 - cornerW2, srcY + cellHeight, cornerW2, cellHeight, options);
        for (px = tlx + cornerW1; px <= trx - cellWidth; px+= cellWidth)
        {
            TE_Img_blitEx(img, src, px, py, srcX + cellWidth, srcY + cellHeight, cellWidth, cellHeight, options);
        }
        if (px < trx)
        {
            TE_Img_blitEx(img, src, px, py, srcX + cellWidth, srcY + cellHeight, trx - px, cellHeight, options);
        }
    }

    if (py < bly)
    {
        TE_Img_blitEx(img, src, tlx, py, srcX, srcY + cellHeight, cornerW1, bly - py, options);
        TE_Img_blitEx(img, src, trx, py, srcX + cellWidth * 3 - cornerW2, srcY + cellHeight, cornerW2, bly - py, options);
        for (px = tlx + cornerW1; px <= trx - cellWidth; px+= cellWidth)
        {
            TE_Img_blitEx(img, src, px, py, srcX + cellWidth, srcY + cellHeight, cellWidth, bly - py, options);
        }
        if (px < trx)
        {
            TE_Img_blitEx(img, src, px, py, srcX + cellWidth, srcY + cellHeight, trx - px, bly - py, options);
        }
    }
}