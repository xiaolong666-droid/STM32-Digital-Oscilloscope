/**
 ******************************************************************************
 * @file    frequency_counter.c
 * @brief   频率测量模块实现
 *
 *          使用 TIM2_CH1 (PA5) 输入捕获功能测量信号频率
 *          原理：捕获连续两个上升沿之间的计数器差值
 *          频率 = 定时器时钟 / (捕获差值)
 *
 *          TIM2 时钟 = APB1 timer clock = 84MHz (APB1=42MHz, timer=84MHz)
 *          对于高频信号（>1kHz），使用无预分频器
 *          对于低频信号，可使用预分频器扩展测量范围
 ******************************************************************************
 */

#include "frequency_counter.h"
#include "oscilloscope.h"

/* External TIM2 handle */
extern TIM_HandleTypeDef htim2;

/* TIM2 clock frequency: APB1 timer clock = 84MHz */
#define TIM2_CLK_FREQ   84000000UL

/* Measurement state */
static volatile uint32_t capture1 = 0;       /* First capture value */
static volatile uint32_t capture2 = 0;       /* Second capture value */
static volatile uint8_t capture_count = 0;   /* Number of captures (0, 1, 2) */
static volatile float measured_freq = 0;     /* Last computed frequency */
static volatile uint8_t freq_valid = 0;      /* Is frequency measurement valid? */
static volatile uint32_t last_capture_time = 0;  /* HAL_GetTick() of last capture */

void frequency_counter_init(void)
{
    capture1 = 0;
    capture2 = 0;
    capture_count = 0;
    measured_freq = 0;
    freq_valid = 0;
    last_capture_time = 0;
}

void frequency_counter_capture_callback(void)
{
    uint32_t current_capture;

    /* Read captured value from TIM2 CCR1 */
    current_capture = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1);

    last_capture_time = HAL_GetTick();

    if (capture_count == 0)
    {
        /* First rising edge captured */
        capture1 = current_capture;
        capture_count = 1;
    }
    else if (capture_count == 1)
    {
        /* Second rising edge captured - compute frequency */
        capture2 = current_capture;
        capture_count = 0;  /* Reset for next measurement */

        /* Calculate period (handle 32-bit counter overflow) */
        uint32_t period;
        if (capture2 >= capture1)
        {
            period = capture2 - capture1;
        }
        else
        {
            /* Counter overflowed (32-bit timer, rare for high frequencies) */
            period = (0xFFFFFFFF - capture1) + capture2 + 1;
        }

        /* Avoid division by zero */
        if (period > 0)
        {
            measured_freq = (float)TIM2_CLK_FREQ / (float)period;
            freq_valid = 1;

            /* Update global oscilloscope config */
            OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();
            cfg->freq_hz = measured_freq;
        }
    }

    /* Reset the counter after second capture for continuous measurement */
    if (capture_count == 0)
    {
        __HAL_TIM_SET_COUNTER(&htim2, 0);
    }
}

float frequency_counter_get_freq(void)
{
    return measured_freq;
}

uint8_t frequency_counter_is_valid(void)
{
    return freq_valid;
}

void frequency_counter_update(void)
{
    /* Check for signal timeout (no capture in 1 second) */
    uint32_t now = HAL_GetTick();
    if (freq_valid && (now - last_capture_time > 1000))
    {
        /* No signal detected for 1 second - mark as invalid */
        freq_valid = 0;
        measured_freq = 0;

        OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();
        cfg->freq_hz = 0;

        /* Reset capture state */
        capture_count = 0;
        __HAL_TIM_SET_COUNTER(&htim2, 0);
    }
}
