/**
 ******************************************************************************
 * @file    fft_analyzer.h
 * @brief   FFT 频谱分析模块
 *
 *          基于 ARM CMSIS-DSP 库实现 1024 点复数 FFT
 *          功能：
 *          - Hanning 窗函数
 *          - 频率峰值检测
 *          - THD（总谐波失真）计算
 ******************************************************************************
 */

#ifndef __FFT_ANALYZER_H
#define __FFT_ANALYZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* FFT configuration */
#define FFT_POINT        1024       /* FFT 点数 */
#define FFT_BINS         (FFT_POINT / 2)  /* 有效频率 bin 数量 (Nyquist) */

/* Window function types */
typedef enum {
    WINDOW_NONE = 0,        /* 矩形窗（无窗） */
    WINDOW_HANNING = 1,     /* 汉宁窗 */
    WINDOW_HAMMING = 2,     /* 汉明窗 */
    WINDOW_BLACKMAN = 3     /* 布莱克曼窗 */
} WindowType_t;

/**
 * @brief  Perform FFT analysis on ADC data
 * @param  adc_data: ADC sample array
 * @param  count: Number of samples (must be <= FFT_POINT)
 * @param  sample_rate: Sample rate in Hz
 * @return 0 on success, -1 on error
 */
int8_t FFT_Analyze(const uint16_t *adc_data, uint16_t count, uint32_t sample_rate);

/**
 * @brief  Get FFT magnitude spectrum
 * @return Pointer to magnitude array (FFT_BINS elements)
 */
float *FFT_GetMagnitude(void);

/**
 * @brief  Get maximum magnitude value
 * @return Max magnitude
 */
float FFT_GetMaxMagnitude(void);

/**
 * @brief  Get frequency of peak bin
 * @return Peak frequency in Hz
 */
float FFT_GetPeakFrequency(void);

/**
 * @brief  Get index of peak bin
 * @return Peak bin index
 */
uint16_t FFT_GetPeakBin(void);

/**
 * @brief  Get THD (Total Harmonic Distortion)
 * @return THD in percent
 */
float FFT_GetTHD(void);

/**
 * @brief  Set window function type
 * @param  type: Window type
 */
void FFT_SetWindow(WindowType_t type);

/**
 * @brief  Get current window type
 * @return Window type
 */
WindowType_t FFT_GetWindow(void);

#ifdef __cplusplus
}
#endif

#endif /* __FFT_ANALYZER_H */
