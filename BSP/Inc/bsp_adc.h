/**
 ******************************************************************************
 * @file    bsp_adc.h
 * @brief   ADC + DMA dual buffer acquisition driver
 *
 *          使用 ADC1 的 2 个规则通道（IN0/IN1），配合 DMA2 Stream0 循环模式
 *          实现最高 1Msps 双通道同步采样，乒乓双缓冲
 ******************************************************************************
 */

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* Buffer parameters */
#define ADC_BUF_HALF_SIZE  (ADC_DMA_BUF_SIZE / 2)  /* Half buffer = 1024 samples */

/* Acquisition status flags */
typedef enum {
    ADC_BUF_EMPTY = 0,
    ADC_BUF_FIRST_HALF_READY = 1,
    ADC_BUF_SECOND_HALF_READY = 2
} ADC_BufferStatus_t;

/* Sample rate settings (controls TIM3 period) */
typedef enum {
    SAMPLE_RATE_1M = 0,     /* 1 Msps */
    SAMPLE_RATE_500K = 1,   /* 500 Ksps */
    SAMPLE_RATE_200K = 2,   /* 200 Ksps */
    SAMPLE_RATE_100K = 3,   /* 100 Ksps */
    SAMPLE_RATE_50K = 4,    /* 50 Ksps */
    SAMPLE_RATE_10K = 5,    /* 10 Ksps */
    SAMPLE_RATE_1K = 6,     /* 1 Ksps */
    SAMPLE_RATE_MAX
} SampleRate_t;

/**
 * @brief  Initialize ADC BSP layer (allocate buffers, set defaults)
 */
void bsp_adc_init(void);

/**
 * @brief  Get pointer to the DMA buffer
 * @return Pointer to uint16_t DMA buffer
 */
uint16_t *bsp_adc_get_buffer(void);

/**
 * @brief  Get the current buffer status (which half is ready)
 * @return ADC_BufferStatus_t
 */
ADC_BufferStatus_t bsp_adc_get_status(void);

/**
 * @brief  Clear the buffer status flag
 */
void bsp_adc_clear_status(void);

/**
 * @brief  Copy the ready half-buffer to a destination array (per channel)
 * @param  ch1_data: Output array for channel 1 (size >= ADC_SAMPLE_NUM)
 * @param  ch2_data: Output array for channel 2 (size >= ADC_SAMPLE_NUM)
 * @return BSP_OK on success
 */
BSP_Status_t bsp_adc_get_data(uint16_t *ch1_data, uint16_t *ch2_data);

/**
 * @brief  Set sample rate by adjusting TIM3 period
 * @param  rate: Sample rate enum
 */
void bsp_adc_set_sample_rate(SampleRate_t rate);

/**
 * @brief  Get current sample rate in Hz
 * @return Sample rate in Hz
 */
uint32_t bsp_adc_get_sample_rate_hz(void);

/**
 * @brief  DMA half-transfer complete callback (called from ISR)
 */
void bsp_adc_half_complete_callback(void);

/**
 * @brief  DMA transfer complete callback (called from ISR)
 */
void bsp_adc_complete_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H */
