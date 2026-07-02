/**
 ******************************************************************************
 * @file    waveform.h
 * @brief   波形处理模块 - 数据缩放、插值、渲染辅助
 ******************************************************************************
 */

#ifndef __WAVEFORM_H
#define __WAVEFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief  Downsample waveform data to fit display width
 * @param  src: Source array
 * @param  src_count: Source sample count
 * @param  dst: Destination array
 * @param  dst_count: Destination sample count
 */
void Waveform_Downsample(const uint16_t *src, uint16_t src_count,
                         uint16_t *dst, uint16_t dst_count);

/**
 * @brief  Linear interpolation between two points
 * @param  x0, y0: First point
 * @param  x1, y1: Second point
 * @param  x: Interpolation X
 * @return Interpolated Y
 */
float Waveform_Lerp(float x0, float y0, float x1, float y1, float x);

/**
 * @brief  Apply moving average filter to waveform data
 * @param  data: Data array (modified in place)
 * @param  count: Number of samples
 * @param  window: Filter window size (must be odd)
 */
void Waveform_MovingAverage(uint16_t *data, uint16_t count, uint8_t window);

/**
 * @brief  Find zero crossings in waveform data
 * @param  data: ADC data array
 * @param  count: Sample count
 * @param  threshold: Zero crossing threshold (ADC value)
 * @param  indices: Output array of crossing indices
 * @param  max_indices: Max number of crossings to find
 * @return Number of crossings found
 */
uint16_t Waveform_FindZeroCrossings(const uint16_t *data, uint16_t count,
                                     uint16_t threshold,
                                     uint16_t *indices, uint16_t max_indices);

#ifdef __cplusplus
}
#endif

#endif /* __WAVEFORM_H */
