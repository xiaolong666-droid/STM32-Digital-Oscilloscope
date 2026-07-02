/**
 ******************************************************************************
 * @file    bsp_adc.c
 * @brief   ADC + DMA 双缓冲采集驱动实现
 *
 *          ADC1 两个规则通道 (IN0/IN1) 以交替扫描模式工作
 *          DMA2 Stream0 循环模式传输，半传输中断(HT)和传输完成中断(TC)
 *          实现乒乓双缓冲：
 *          - HT: 前半段数据就绪 (buf[0..1023])
 *          - TC: 后半段数据就绪 (buf[1024..2047])
 *          每半段包含 1024 个采样点，交替存储 CH1/CH2
 ******************************************************************************
 */

#include "bsp_adc.h"

/* DMA buffer: 2048 half-words (1024 per channel, interleaved)
 * Layout: [CH1_0, CH2_0, CH1_1, CH2_1, ..., CH1_1023, CH2_1023]
 * Half 1: indices 0..1023 (512 CH1 + 512 CH2 samples)
 * Half 2: indices 1024..2047 (512 CH1 + 512 CH2 samples)
 *
 * Note: With 2 channels and NbrOfConversion=2, each DMA transfer
 * moves 2 half-words (one per channel). Total buffer = 1024 * 2 = 2048.
 */
static uint16_t adc_dma_buffer[ADC_DMA_BUF_SIZE] __attribute__((aligned(32)));

/* Buffer status - set by DMA callbacks */
static volatile ADC_BufferStatus_t adc_buf_status = ADC_BUF_EMPTY;

/* Sample rate configuration table
 * TIM3 clock = APB1 timer clock = 84MHz * 2 = 84MHz (APB1=42MHz, timer=84MHz)
 * Period = TIM3_CLK / sample_rate
 */
static const struct {
    uint32_t rate_hz;    /* Actual sample rate in Hz */
    uint16_t tim_period; /* TIM3 auto-reload value */
} sample_rate_table[SAMPLE_RATE_MAX] = {
    { 1000000, 83 },   /* 1 Msps:  84MHz / 84 = 1MHz */
    { 500000,  167 },  /* 500 Ksps: 84MHz / 168 = 500KHz */
    { 200000,  419 },  /* 200 Ksps: 84MHz / 420 ≈ 200KHz */
    { 100000,  839 },  /* 100 Ksps: 84MHz / 840 = 100KHz */
    { 50000,   1679 }, /* 50 Ksps:  84MHz / 1680 = 50KHz */
    { 10000,   8399 }, /* 10 Ksps:  84MHz / 8400 = 10KHz */
    { 1000,    83999 } /* 1 Ksps:   84MHz / 84000 = 1KHz */
};

static SampleRate_t current_rate = SAMPLE_RATE_1M;

void bsp_adc_init(void)
{
    /* Buffer is statically allocated, just reset status */
    adc_buf_status = ADC_BUF_EMPTY;
    current_rate = SAMPLE_RATE_1M;

    /* TIM3 period will be set in set_sample_rate */
    bsp_adc_set_sample_rate(SAMPLE_RATE_1M);
}

uint16_t *bsp_adc_get_buffer(void)
{
    return adc_dma_buffer;
}

ADC_BufferStatus_t bsp_adc_get_status(void)
{
    return adc_buf_status;
}

void bsp_adc_clear_status(void)
{
    adc_buf_status = ADC_BUF_EMPTY;
}

BSP_Status_t bsp_adc_get_data(uint16_t *ch1_data, uint16_t *ch2_data)
{
    uint16_t *src;
    uint32_t offset;
    uint32_t i;

    if (adc_buf_status == ADC_BUF_EMPTY)
    {
        return BSP_ERROR;
    }

    /* Determine which half to read */
    if (adc_buf_status == ADC_BUF_FIRST_HALF_READY)
    {
        offset = 0;
    }
    else
    {
        offset = ADC_BUF_HALF_SIZE;
    }

    src = &adc_dma_buffer[offset];

    /* De-interleave: [CH1, CH2, CH1, CH2, ...] -> separate arrays
     * Each half has ADC_BUF_HALF_SIZE/2 = 512 samples per channel
     */
    uint32_t samples_per_ch = ADC_BUF_HALF_SIZE / ADC_CHANNELS;
    for (i = 0; i < samples_per_ch; i++)
    {
        ch1_data[i] = src[i * ADC_CHANNELS];
        ch2_data[i] = src[i * ADC_CHANNELS + 1];
    }

    adc_buf_status = ADC_BUF_EMPTY;
    return BSP_OK;
}

void bsp_adc_set_sample_rate(SampleRate_t rate)
{
    if (rate >= SAMPLE_RATE_MAX)
    {
        return;
    }

    current_rate = rate;

    /* Update TIM3 auto-reload register directly */
    extern TIM_HandleTypeDef htim3;
    __HAL_TIM_SET_AUTORELOAD(&htim3, sample_rate_table[rate].tim_period);
}

uint32_t bsp_adc_get_sample_rate_hz(void)
{
    return sample_rate_table[current_rate].rate_hz;
}

/* === DMA Callbacks (called from ISR context) === */

void bsp_adc_half_complete_callback(void)
{
    /* First half of DMA buffer is filled */
    adc_buf_status = ADC_BUF_FIRST_HALF_READY;
}

void bsp_adc_complete_callback(void)
{
    /* Second half of DMA buffer is filled */
    adc_buf_status = ADC_BUF_SECOND_HALF_READY;
}
