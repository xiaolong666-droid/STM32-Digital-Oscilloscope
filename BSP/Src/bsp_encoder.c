/**
 ******************************************************************************
 * @file    bsp_encoder.c
 * @brief   旋转编码器驱动实现
 *
 *          使用 TIM4 编码器接口模式读取 AB 相信号 (PB6/PB7)
 *          编码器按键 (PB8) 通过外部中断触发，TIM6 定时器消抖
 ******************************************************************************
 */

#include "bsp_encoder.h"

/* External TIM4 handle */
extern TIM_HandleTypeDef htim4;

/* Encoder state */
static volatile int16_t enc_last_count = 0;     /* Last read counter value */
static volatile int16_t enc_delta = 0;           /* Accumulated delta */

/* Button state (debounced) */
static volatile uint8_t btn_raw_state = 1;        /* Raw pin state (active low) */
static volatile uint8_t btn_debounced = 1;        /* Debounced state */
static volatile uint8_t btn_debounce_counter = 0; /* Debounce counter */
static volatile uint8_t btn_clicked_flag = 0;     /* Click event flag */
static volatile uint8_t btn_int_triggered = 0;    /* EXTI triggered flag */

void bsp_encoder_init(void)
{
    enc_last_count = 0;
    enc_delta = 0;
    btn_raw_state = 1;
    btn_debounced = 1;
    btn_debounce_counter = 0;
    btn_clicked_flag = 0;
    btn_int_triggered = 0;

    /* Reset TIM4 counter */
    __HAL_TIM_SET_COUNTER(&htim4, 0);
}

int16_t bsp_encoder_get_delta(void)
{
    int16_t current = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    int16_t delta = current - enc_last_count;
    enc_last_count = current;

    /* Handle 16-bit counter overflow/underflow */
    if (delta > 32767)
    {
        delta -= 65536;
    }
    else if (delta < -32768)
    {
        delta += 65536;
    }

    /* Divide by 4 (quadrature encoder gives 4 counts per detent) */
    delta /= 4;

    enc_delta += delta;
    int16_t result = enc_delta;
    enc_delta = 0;

    return result;
}

int16_t bsp_encoder_get_count(void)
{
    return (int16_t)__HAL_TIM_GET_COUNTER(&htim4) / 4;
}

void bsp_encoder_reset(void)
{
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    enc_last_count = 0;
    enc_delta = 0;
}

uint8_t bsp_encoder_button_pressed(void)
{
    return (btn_debounced == 0) ? 1 : 0;
}

uint8_t bsp_encoder_button_clicked(void)
{
    if (btn_clicked_flag)
    {
        btn_clicked_flag = 0;
        return 1;
    }
    return 0;
}

void bsp_encoder_btn_callback(void)
{
    /* Set flag - actual debounce is done in TIM6 ISR */
    btn_int_triggered = 1;
}

void bsp_encoder_debounce_process(void)
{
    uint8_t current_pin;

    if (!btn_int_triggered)
    {
        return;
    }

    /* Read actual pin state */
    current_pin = HAL_GPIO_ReadPin(ENC_PORT, ENC_BTN_PIN);

    if (current_pin != btn_raw_state)
    {
        btn_raw_state = current_pin;
        btn_debounce_counter = 0;
        return;
    }

    btn_debounce_counter++;

    /* After 3 consecutive matches (30ms), accept as stable */
    if (btn_debounce_counter >= 3)
    {
        btn_debounce_counter = 0;

        if (btn_raw_state != btn_debounced)
        {
            btn_debounced = btn_raw_state;

            /* Detect click: button released (active low: 0->1 transition) */
            if (btn_debounced == 1)
            {
                btn_clicked_flag = 1;
            }
        }

        /* Clear the interrupt trigger flag */
        btn_int_triggered = 0;
    }
}
