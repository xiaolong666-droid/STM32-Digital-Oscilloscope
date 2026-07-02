/**
 ******************************************************************************
 * @file    bsp_timer.c
 * @brief   定时器工具函数实现
 *
 *          TIM6 作为 10ms 消抖定时器，统一管理编码器和按键的消抖
 ******************************************************************************
 */

#include "bsp_timer.h"
#include "bsp_encoder.h"
#include "bsp_button.h"

void bsp_timer_init(void)
{
    /* TIM6 is already started in main.c with interrupt */
    /* Nothing else to do here */
}

uint32_t bsp_timer_get_ms(void)
{
    return HAL_GetTick();
}

uint8_t bsp_timer_elapsed(uint32_t start, uint32_t timeout_ms)
{
    uint32_t now = HAL_GetTick();
    return ((now - start) >= timeout_ms) ? 1 : 0;
}

void bsp_timer_debounce_callback(void)
{
    /* This is called from TIM6_DAC_IRQHandler at 10ms intervals */
    /* Process encoder debounce */
    bsp_encoder_debounce_process();

    /* Process button debounce */
    bsp_button_debounce_process();
}
