/**
 ******************************************************************************
 * @file    lcd_font.h
 * @brief   ASCII font definitions for LCD display
 *          8x16 and 16x24 fonts
 ******************************************************************************
 */

#ifndef __LCD_FONT_H
#define __LCD_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Font sizes */
typedef enum {
    FONT_SMALL = 0,    /* 8x16  pixels */
    FONT_MEDIUM = 1,   /* 12x20 pixels */
    FONT_LARGE = 2     /* 16x24 pixels */
} FontSize_t;

/* Font dimensions */
#define FONT_SMALL_W    8
#define FONT_SMALL_H    16
#define FONT_MEDIUM_W   12
#define FONT_MEDIUM_H   20
#define FONT_LARGE_W    16
#define FONT_LARGE_H    24

/**
 * @brief  Get font bitmap data for a character
 * @param  ch: ASCII character (0x20 - 0x7E)
 * @param  size: Font size
 * @return Pointer to bitmap data, NULL if not available
 */
const uint8_t *LCD_Font_GetChar(char ch, FontSize_t size);

/**
 * @brief  Get font width for a given size
 * @param  size: Font size
 * @return Width in pixels
 */
uint8_t LCD_Font_GetWidth(FontSize_t size);

/**
 * @brief  Get font height for a given size
 * @param  size: Font size
 * @return Height in pixels
 */
uint8_t LCD_Font_GetHeight(FontSize_t size);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_FONT_H */
