/**
 ******************************************************************************
 * @file    lcd_graphic.c
 * @brief   LCD 图形库实现
 *          提供基本图形绘制：画点、画线、矩形、圆、文字、网格背景
 *          基于这些原语实现示波器的波形显示和界面绘制
 ******************************************************************************
 */

#include "lcd_graphic.h"

/* Current text colors (module-internal) */
static uint16_t text_color = COLOR_WHITE;
static uint16_t text_bg_color = COLOR_BLACK;

/* ======================== Drawing Primitives ======================== */

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
        return;
    ILI9341_DrawPixel(x, y, color);
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t dx, dy, sx, sy, err, e2;

    /* Clip coordinates */
    if (x1 >= LCD_WIDTH) x1 = LCD_WIDTH - 1;
    if (x2 >= LCD_WIDTH) x2 = LCD_WIDTH - 1;
    if (y1 >= LCD_HEIGHT) y1 = LCD_HEIGHT - 1;
    if (y2 >= LCD_HEIGHT) y2 = LCD_HEIGHT - 1;

    dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;

    /* Bresenham line algorithm */
    while (1)
    {
        ILI9341_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2)
            break;
        e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

void LCD_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    if (y >= LCD_HEIGHT)
        return;
    if (x + w > LCD_WIDTH)
        w = LCD_WIDTH - x;

    ILI9341_FillRect(x, y, w, 1, color);
}

void LCD_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
    if (x >= LCD_WIDTH)
        return;
    if (y + h > LCD_HEIGHT)
        h = LCD_HEIGHT - y;

    ILI9341_FillRect(x, y, 1, h, color);
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    LCD_DrawHLine(x, y, w, color);           /* Top */
    LCD_DrawHLine(x, y + h - 1, w, color);   /* Bottom */
    LCD_DrawVLine(x, y, h, color);           /* Left */
    LCD_DrawVLine(x + w - 1, y, h, color);   /* Right */
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
        return;
    if (x + w > LCD_WIDTH)
        w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT)
        h = LCD_HEIGHT - y;

    ILI9341_FillRect(x, y, w, h, color);
}

void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    ILI9341_DrawPixel(x0, y0 + r, color);
    ILI9341_DrawPixel(x0, y0 - r, color);
    ILI9341_DrawPixel(x0 + r, y0, color);
    ILI9341_DrawPixel(x0 - r, y0, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ILI9341_DrawPixel(x0 + x, y0 + y, color);
        ILI9341_DrawPixel(x0 - x, y0 + y, color);
        ILI9341_DrawPixel(x0 + x, y0 - y, color);
        ILI9341_DrawPixel(x0 - x, y0 - y, color);
        ILI9341_DrawPixel(x0 + y, y0 + x, color);
        ILI9341_DrawPixel(x0 - y, y0 + x, color);
        ILI9341_DrawPixel(x0 + y, y0 - x, color);
        ILI9341_DrawPixel(x0 - y, y0 - x, color);
    }
}

void LCD_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t i;

    /* Draw vertical lines from top to bottom */
    for (i = y0 - r; i <= y0 + r; i++)
    {
        ILI9341_DrawPixel(x0, i, color);
    }

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        for (i = y0 - y; i <= y0 + y; i++)
        {
            ILI9341_DrawPixel(x0 + x, i, color);
            ILI9341_DrawPixel(x0 - x, i, color);
        }
        for (i = y0 - x; i <= y0 + x; i++)
        {
            ILI9341_DrawPixel(x0 + y, i, color);
            ILI9341_DrawPixel(x0 - y, i, color);
        }
    }
}

/* ======================== Text Functions ======================== */

void LCD_SetTextColor(uint16_t color)
{
    text_color = color;
}

void LCD_SetBackColor(uint16_t color)
{
    text_bg_color = color;
}

void LCD_SetFontColor(uint16_t fg, uint16_t bg)
{
    text_color = fg;
    text_bg_color = bg;
}

uint16_t LCD_GetTextColor(void)
{
    return text_color;
}

uint16_t LCD_GetBackColor(void)
{
    return text_bg_color;
}

void LCD_DrawChar(uint16_t x, uint16_t y, char ch, FontSize_t size, uint16_t fg, uint16_t bg)
{
    const uint8_t *bitmap;
    uint8_t font_w, font_h;
    uint8_t row, col;
    uint8_t pixel;
    uint8_t scale;

    /* Determine scale factor */
    switch (size)
    {
    case FONT_SMALL:  scale = 1; break;
    case FONT_MEDIUM: scale = 2; break;
    case FONT_LARGE:  scale = 3; break;  /* Actually 2x, adjust below */
    default:          scale = 1; break;
    }

    /* Use base 8x16 font, scale up */
    bitmap = LCD_Font_GetChar(ch, FONT_SMALL);
    font_w = FONT_SMALL_W;  /* 8 */
    font_h = FONT_SMALL_H;  /* 16 */

    /* Set window for the character */
    uint16_t w = font_w * scale;
    uint16_t h = font_h * scale;

    if (x + w > LCD_WIDTH || y + h > LCD_HEIGHT)
        return;

    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);

    /* Draw each pixel (scaled) */
    for (row = 0; row < font_h; row++)
    {
        for (uint8_t sy = 0; sy < scale; sy++)
        {
            for (col = 0; col < font_w; col++)
            {
                pixel = (bitmap[row] >> (7 - col)) & 0x01;
                uint16_t color = pixel ? fg : bg;
                for (uint8_t sx = 0; sx < scale; sx++)
                {
                    *LCD_DATA_ADDR = color;
                }
            }
        }
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, const char *str, FontSize_t size, uint16_t fg, uint16_t bg)
{
    uint8_t char_w = LCD_Font_GetWidth(size);
    uint16_t cur_x = x;

    while (*str)
    {
        LCD_DrawChar(cur_x, y, *str, size, fg, bg);
        cur_x += char_w;
        str++;
    }
}

void LCD_DrawStringCenter(uint16_t x, uint16_t y, uint16_t w, const char *str,
                          FontSize_t size, uint16_t fg, uint16_t bg)
{
    uint8_t char_w = LCD_Font_GetWidth(size);
    uint16_t str_len = 0;
    const char *p = str;

    while (*p++)
        str_len++;

    uint16_t text_w = str_len * char_w;
    uint16_t start_x = x + (w - text_w) / 2;

    LCD_DrawString(start_x, y, str, size, fg, bg);
}

void LCD_Clear(uint16_t color)
{
    ILI9341_FillScreen(color);
}

/* ======================== Grid Drawing ======================== */

void LCD_DrawGrid(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  uint8_t div_x, uint8_t div_y, uint16_t color)
{
    uint16_t i;
    uint16_t step_x = w / div_x;
    uint16_t step_y = h / div_y;

    /* Draw vertical grid lines */
    for (i = 0; i <= div_x; i++)
    {
        uint16_t line_x = x + i * step_x;
        if (line_x >= x + w)
            line_x = x + w - 1;

        /* Draw dotted/dashed line for better waveform visibility */
        for (uint16_t yy = y; yy < y + h; yy += 2)
        {
            ILI9341_DrawPixel(line_x, yy, color);
        }
    }

    /* Draw horizontal grid lines */
    for (i = 0; i <= div_y; i++)
    {
        uint16_t line_y = y + i * step_y;
        if (line_y >= y + h)
            line_y = y + h - 1;

        for (uint16_t xx = x; xx < x + w; xx += 2)
        {
            ILI9341_DrawPixel(xx, line_y, color);
        }
    }

    /* Draw center crosshair (solid, brighter) */
    uint16_t cx = x + w / 2;
    uint16_t cy = y + h / 2;

    LCD_DrawVLine(cx, y, h, color);
    LCD_DrawHLine(x, cy, w, color);
}
