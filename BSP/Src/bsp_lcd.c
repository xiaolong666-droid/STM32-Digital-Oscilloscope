/**
 ******************************************************************************
 * @file    bsp_lcd.c
 * @brief   LCD BSP layer implementation - FSMC interface for ILI9341
 *
 *          通过 FSMC Bank1 (NE1) 驱动 ILI9341 TFT-LCD
 *          16位数据总线，A16 作为命令/数据选择 (RS)
 *          地址映射：
 *            RS=0 (命令): 0x60000000
 *            RS=1 (数据): 0x60020000
 ******************************************************************************
 */

#include "bsp_lcd.h"

void bsp_lcd_init(void)
{
    /* Hardware reset sequence */
    bsp_lcd_reset();

    /* Backlight on */
    bsp_lcd_set_backlight(1);
}

void bsp_lcd_write_cmd(uint16_t cmd)
{
    *LCD_CMD_ADDR = cmd;
}

void bsp_lcd_write_data(uint16_t data)
{
    *LCD_DATA_ADDR = data;
}

uint16_t bsp_lcd_read_data(void)
{
    return *LCD_DATA_ADDR;
}

void bsp_lcd_write_data_multi(uint16_t data, uint32_t count)
{
    while (count--)
    {
        *LCD_DATA_ADDR = data;
    }
}

void bsp_lcd_reset(void)
{
    /* Pull reset low then high */
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
}

void bsp_lcd_set_backlight(uint8_t on)
{
    if (on)
    {
        HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);
    }
}
