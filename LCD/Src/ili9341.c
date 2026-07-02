/**
 ******************************************************************************
 * @file    ili9341.c
 * @brief   ILI9341 TFT-LCD 控制器驱动实现
 *
 *          通过 FSMC 8080 接口驱动 ILI9341 控制器
 *          支持 320x240 分辨率，16位色 (RGB565)
 *          实现窗口设置、屏幕填充、像素绘制等功能
 ******************************************************************************
 */

#include "ili9341.h"

/* Current rotation state */
static uint8_t current_rotation = 1;  /* Default: landscape */
static uint16_t lcd_width = LCD_WIDTH;   /* 320 */
static uint16_t lcd_height = LCD_HEIGHT; /* 240 */

/* ====== Low-level register access ====== */

static void ILI9341_WriteReg(uint16_t cmd)
{
    *LCD_CMD_ADDR = cmd;
}

static void ILI9341_WriteData16(uint16_t data)
{
    *LCD_DATA_ADDR = data;
}

static uint16_t ILI9341_ReadData16(void)
{
    return *LCD_DATA_ADDR;
}

static void ILI9341_WriteCmdData(uint16_t cmd, uint16_t data)
{
    ILI9341_WriteReg(cmd);
    ILI9341_WriteData16(data);
}

/* ====== Initialization sequence ====== */

void ILI9341_Init(void)
{
    /* Hardware reset is done in bsp_lcd_init, but do software reset too */
    ILI9341_WriteReg(ILI9341_SWRESET);
    HAL_Delay(5);

    /* Sleep out */
    ILI9341_WriteReg(ILI9341_SLPOUT);
    HAL_Delay(120);

    /* Pixel Format: 16 bits per pixel (RGB565) */
    ILI9341_WriteCmdData(ILI9341_PIXFMT, 0x55);

    /* Power Control 1: GVDD = 4.6V */
    ILI9341_WriteReg(ILI9341_PWCTR1);
    ILI9341_WriteData16(0x23);

    /* Power Control 2: VGH = 2xGVDD, VGL = -2xGVDD */
    ILI9341_WriteReg(ILI9341_PWCTR2);
    ILI9341_WriteData16(0x10);

    /* VCOM Control 1: VCOMH = 3.85V, VCOML = -1.55V */
    ILI9341_WriteReg(ILI9341_VMCTR1);
    ILI9341_WriteData16(0x2B);
    ILI9341_WriteData16(0x2B);

    /* VCOM Offset */
    ILI9341_WriteReg(ILI9341_VMOFCTR);
    ILI9341_WriteData16(0x40);

    /* Memory Access Control: MX + RGB (landscape mode default) */
    ILI9341_SetRotation(1);

    /* Frame Rate Control: Division=1, Frame Rate=79Hz normal mode */
    ILI9341_WriteReg(ILI9341_FRMCTR1);
    ILI9341_WriteData16(0x00);
    ILI9341_WriteData16(0x1B);

    /* Display Function Control */
    ILI9341_WriteReg(ILI9341_DFUNCTR);
    ILI9341_WriteData16(0x0A);
    ILI9341_WriteData16(0xA2);

    /* Enable 3G gamma */
    ILI9341_WriteCmdData(ILI9341_GAMMASET, 0x01);

    /* Positive Gamma Correction */
    ILI9341_WriteReg(ILI9341_GMCTRP1);
    const uint8_t pgamma[] = {0x0F, 0x1D, 0x1A, 0x0A, 0x0D, 0x07, 0x49, 0x66,
                              0x3C, 0x04, 0x0E, 0x02, 0x00, 0x2F, 0x3F};
    uint8_t i;
    for (i = 0; i < 15; i++)
    {
        ILI9341_WriteData16(pgamma[i]);
    }

    /* Negative Gamma Correction */
    ILI9341_WriteReg(ILI9341_GMCTRN1);
    const uint8_t ngamma[] = {0x00, 0x22, 0x25, 0x05, 0x12, 0x08, 0x36, 0x19,
                              0x43, 0x0B, 0x11, 0x0D, 0x3F, 0x10, 0x00};
    for (i = 0; i < 15; i++)
    {
        ILI9341_WriteData16(ngamma[i]);
    }

    /* Sleep out again */
    ILI9341_WriteReg(ILI9341_SLPOUT);
    HAL_Delay(120);

    /* Display ON */
    ILI9341_WriteReg(ILI9341_DISPON);

    /* Clear screen */
    ILI9341_FillScreen(COLOR_BLACK);
}

void ILI9341_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* Column address set */
    ILI9341_WriteReg(ILI9341_CASET);
    ILI9341_WriteData16(x1 >> 8);
    ILI9341_WriteData16(x1 & 0xFF);
    ILI9341_WriteData16(x2 >> 8);
    ILI9341_WriteData16(x2 & 0xFF);

    /* Page address set */
    ILI9341_WriteReg(ILI9341_PASET);
    ILI9341_WriteData16(y1 >> 8);
    ILI9341_WriteData16(y1 & 0xFF);
    ILI9341_WriteData16(y2 >> 8);
    ILI9341_WriteData16(y2 & 0xFF);

    /* Write to RAM */
    ILI9341_WriteReg(ILI9341_RAMWR);
}

void ILI9341_SetRotation(uint8_t rotation)
{
    current_rotation = rotation & 0x03;

    ILI9341_WriteReg(ILI9341_MADCTL);

    switch (current_rotation)
    {
    case 0:  /* Portrait */
        ILI9341_WriteData16(MADCTL_MX | MADCTL_RGB);
        lcd_width = 240;
        lcd_height = 320;
        break;
    case 1:  /* Landscape */
        ILI9341_WriteData16(MADCTL_MV | MADCTL_MX | MADCTL_RGB);
        lcd_width = 320;
        lcd_height = 240;
        break;
    case 2:  /* Portrait flip */
        ILI9341_WriteData16(MADCTL_MY | MADCTL_MX | MADCTL_RGB);
        lcd_width = 240;
        lcd_height = 320;
        break;
    case 3:  /* Landscape flip */
        ILI9341_WriteData16(MADCTL_MV | MADCTL_MY | MADCTL_MX | MADCTL_RGB);
        lcd_width = 320;
        lcd_height = 240;
        break;
    }
}

void ILI9341_FillScreen(uint16_t color)
{
    ILI9341_FillRect(0, 0, lcd_width - 1, lcd_height - 1, color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint32_t total = (uint32_t)w * h;

    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);

    /* Write pixel data */
    while (total--)
    {
        *LCD_DATA_ADDR = color;
    }
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    ILI9341_SetWindow(x, y, x, y);
    *LCD_DATA_ADDR = color;
}

uint16_t ILI9341_ReadPixel(uint16_t x, uint16_t y)
{
    ILI9341_SetWindow(x, y, x, y);
    ILI9341_WriteReg(ILI9341_RAMRD);

    /* Dummy read */
    ILI9341_ReadData16();
    /* Actual pixel data */
    return ILI9341_ReadData16();
}

uint16_t ILI9341_GetWidth(void)
{
    return lcd_width;
}

uint16_t ILI9341_GetHeight(void)
{
    return lcd_height;
}

uint16_t ILI9341_Color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
