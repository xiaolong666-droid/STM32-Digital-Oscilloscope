/**
 ******************************************************************************
 * @file    waveform.c
 * @brief   波形处理模块实现
 ******************************************************************************
 */

#include "waveform.h"
#include <string.h>

void Waveform_Downsample(const uint16_t *src, uint16_t src_count,
                         uint16_t *dst, uint16_t dst_count)
{
    uint16_t i;
    float ratio;

    if (src_count == 0 || dst_count == 0)
        return;

    if (src_count <= dst_count)
    {
        /* Upsample or direct copy */
        memcpy(dst, src, src_count * sizeof(uint16_t));
        return;
    }

    ratio = (float)src_count / (float)dst_count;

    for (i = 0; i < dst_count; i++)
    {
        uint16_t src_idx = (uint16_t)(i * ratio);
        if (src_idx >= src_count)
            src_idx = src_count - 1;
        dst[i] = src[src_idx];
    }
}

float Waveform_Lerp(float x0, float y0, float x1, float y1, float x)
{
    if (x1 == x0)
        return y0;
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

void Waveform_MovingAverage(uint16_t *data, uint16_t count, uint8_t window)
{
    if (window < 3 || window % 2 == 0)
        window = 3;  /* Must be odd, minimum 3 */

    uint8_t half = window / 2;
    uint16_t i;
    int32_t j;
    static uint16_t temp[ADC_SAMPLE_NUM];  /* Static buffer to avoid malloc */

    if (count > ADC_SAMPLE_NUM)
        return;

    for (i = 0; i < count; i++)
    {
        uint32_t sum = 0;
        uint16_t cnt = 0;

        for (j = -half; j <= half; j++)
        {
            int32_t idx = (int32_t)i + j;
            if (idx >= 0 && idx < count)
            {
                sum += data[idx];
                cnt++;
            }
        }
        temp[i] = (uint16_t)(sum / cnt);
    }

    memcpy(data, temp, count * sizeof(uint16_t));
}

uint16_t Waveform_FindZeroCrossings(const uint16_t *data, uint16_t count,
                                     uint16_t threshold,
                                     uint16_t *indices, uint16_t max_indices)
{
    uint16_t i;
    uint16_t found = 0;

    for (i = 0; i < count - 1 && found < max_indices; i++)
    {
        /* Detect crossing: one below threshold, next above (rising) */
        if (data[i] < threshold && data[i + 1] >= threshold)
        {
            indices[found++] = i;
        }
    }

    return found;
}
