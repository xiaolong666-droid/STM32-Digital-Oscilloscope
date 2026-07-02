/**
 ******************************************************************************
 * @file    lcd_graphic.h
 * @brief   LCD graphics library - drawing primitives, text, grid
 ******************************************************************************
 */

#ifndef __LCD_GRAPHIC_H
#define __LCD_GRAPHIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9341.h"
#include "lcd_font.h"

/* Drawing primitives */
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void LCD_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void LCD_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

/* Text functions */
void LCD_SetTextColor(uint16_t color);
void LCD_SetBackColor(uint16_t color);
void LCD_SetFontColor(uint16_t fg, uint16_t bg);
void LCD_DrawChar(uint16_t x, uint16_t y, char ch, FontSize_t size, uint16_t fg, uint16_t bg);
void LCD_DrawString(uint16_t x, uint16_t y, const char *str, FontSize_t size, uint16_t fg, uint16_t bg);
void LCD_DrawStringCenter(uint16_t x, uint16_t y, uint16_t w, const char *str, FontSize_t size, uint16_t fg, uint16_t bg);

/* Utility functions */
void LCD_Clear(uint16_t color);
uint16_t LCD_GetTextColor(void);
uint16_t LCD_GetBackColor(void);

/* Grid drawing for oscilloscope */
void LCD_DrawGrid(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  uint8_t div_x, uint8_t div_y, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_GRAPHIC_H */
