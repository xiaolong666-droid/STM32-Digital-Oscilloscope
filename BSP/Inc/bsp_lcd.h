/**
 ******************************************************************************
 * @file    bsp_lcd.h
 * @brief   LCD BSP layer - FSMC interface abstraction
 ******************************************************************************
 */

#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* FSMC Bank1 address mapping for NE1 (CS) with A16 as RS:
 * Base address for NE1 = 0x60000000
 * With A16 (bit 16), RS=0 -> 0x60000000 (command), RS=1 -> 0x60020000 (data)
 */
#define LCD_BASE           0x60000000U
#define LCD_CMD_ADDR       ((__IO uint16_t *)(LCD_BASE))
#define LCD_DATA_ADDR      ((__IO uint16_t *)(LCD_BASE + (1 << 16)))

/**
 * @brief  Initialize LCD BSP (hardware reset, backlight)
 */
void bsp_lcd_init(void);

/**
 * @brief  Write a command to LCD
 * @param  cmd: 16-bit command
 */
void bsp_lcd_write_cmd(uint16_t cmd);

/**
 * @brief  Write data to LCD
 * @param  data: 16-bit data
 */
void bsp_lcd_write_data(uint16_t data);

/**
 * @brief  Read data from LCD
 * @return 16-bit data
 */
uint16_t bsp_lcd_read_data(void);

/**
 * @brief  Write multiple data words (for block fill)
 * @param  data: data to write
 * @param  count: number of writes
 */
void bsp_lcd_write_data_multi(uint16_t data, uint32_t count);

/**
 * @brief  Hardware reset LCD
 */
void bsp_lcd_reset(void);

/**
 * @brief  Set backlight on/off
 * @param  on: 1=on, 0=off
 */
void bsp_lcd_set_backlight(uint8_t on);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LCD_H */
