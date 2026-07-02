/**
 ******************************************************************************
 * @file    ili9341.h
 * @brief   ILI9341 TFT-LCD controller driver
 *          320x240 resolution, 16-bit color (RGB565), FSMC interface
 ******************************************************************************
 */

#ifndef __ILI9341_H
#define __ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_lcd.h"

/* Color definitions (RGB565) */
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_YELLOW      0xFFE0
#define COLOR_ORANGE      0xFD20
#define COLOR_GRAY        0x8410
#define COLOR_DARKGRAY    0x4208
#define COLOR_LIGHTGRAY   0xC618
#define COLOR_DARKGREEN   0x03E0
#define COLOR_DARKRED     0x8000
#define COLOR_NAVY        0x000F
#define COLOR_PURPLE      0x780F

/* ILI9341 command set */
#define ILI9341_NOP       0x00
#define ILI9341_SWRESET   0x01
#define ILI9341_RDDID     0x04
#define ILI9341_RDDST     0x09

#define ILI9341_SLPIN     0x10
#define ILI9341_SLPOUT    0x11
#define ILI9341_PTLON     0x12
#define ILI9341_NORON     0x13

#define ILI9341_RDMODE    0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0D
#define ILI9341_RDSELFDIAG 0x0F

#define ILI9341_INVOFF    0x20
#define ILI9341_INVON     0x21
#define ILI9341_GAMMASET  0x26
#define ILI9341_DISPOFF   0x28
#define ILI9341_DISPON    0x29

#define ILI9341_CASET     0x2A
#define ILI9341_PASET     0x2B
#define ILI9341_RAMWR     0x2C
#define ILI9341_RAMRD     0x2E

#define ILI9341_PTLAR     0x30
#define ILI9341_VSCRDEF   0x33
#define ILI9341_MADCTL    0x36
#define ILI9341_VSCRSADD  0x37
#define ILI9341_PIXFMT    0x3A

#define ILI9341_FRMCTR1   0xB1
#define ILI9341_FRMCTR2   0xB2
#define ILI9341_FRMCTR3   0xB3
#define ILI9341_INVCTR    0xB4
#define ILI9341_DFUNCTR   0xB6

#define ILI9341_PWCTR1    0xC0
#define ILI9341_PWCTR2    0xC1
#define ILI9341_PWCTR3    0xC2
#define ILI9341_PWCTR4    0xC3
#define ILI9341_PWCTR5    0xC4
#define ILI9341_VMCTR1    0xC5
#define ILI9341_VMOFCTR   0xC7

#define ILI9341_RDID1     0xDA
#define ILI9341_RDID2     0xDB
#define ILI9341_RDID3     0xDC
#define ILI9341_RDID4     0xDD

#define ILI9341_GMCTRP1   0xE0
#define ILI9341_GMCTRN1   0xE1

/* MADCTL bits */
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x08
#define MADCTL_MH  0x04

/**
 * @brief  Initialize ILI9341 LCD controller
 */
void ILI9341_Init(void);

/**
 * @brief  Set the drawing window (column and page address)
 * @param  x1, y1: Top-left corner
 * @param  x2, y2: Bottom-right corner
 */
void ILI9341_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief  Set rotation/orientation
 * @param  rotation: 0=portrait, 1=landscape, 2=portrait-flip, 3=landscape-flip
 */
void ILI9341_SetRotation(uint8_t rotation);

/**
 * @brief  Fill entire screen with a color
 * @param  color: RGB565 color
 */
void ILI9341_FillScreen(uint16_t color);

/**
 * @brief  Fill a rectangular area
 * @param  x, y: Top-left corner
 * @param  w, h: Width and height
 * @param  color: Fill color
 */
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief  Draw a single pixel
 * @param  x, y: Coordinates
 * @param  color: RGB565 color
 */
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  Get pixel color at coordinates
 * @param  x, y: Coordinates
 * @return RGB565 color
 */
uint16_t ILI9341_ReadPixel(uint16_t x, uint16_t y);

/**
 * @brief  Get LCD width (depends on rotation)
 * @return Width in pixels
 */
uint16_t ILI9341_GetWidth(void);

/**
 * @brief  Get LCD height (depends on rotation)
 * @return Height in pixels
 */
uint16_t ILI9341_GetHeight(void);

/**
 * @brief  Convert 24-bit RGB to 16-bit RGB565
 * @param  r, g, b: 8-bit color components
 * @return RGB565 color
 */
uint16_t ILI9341_Color565(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_H */
