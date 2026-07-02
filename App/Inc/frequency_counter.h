/**
 ******************************************************************************
 * @file    frequency_counter.h
 * @brief   频率测量模块 - 基于 TIM2 输入捕获
 ******************************************************************************
 */

#ifndef __FREQUENCY_COUNTER_H
#define __FREQUENCY_COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief  Initialize frequency counter
 */
void frequency_counter_init(void);

/**
 * @brief  Input capture callback (called from TIM2 ISR)
 *         Measures time between rising edges to compute frequency
 */
void frequency_counter_capture_callback(void);

/**
 * @brief  Get measured frequency
 * @return Frequency in Hz (0 if no signal detected)
 */
float frequency_counter_get_freq(void);

/**
 * @brief  Check if frequency measurement is valid
 * @return 1 if valid, 0 if no signal
 */
uint8_t frequency_counter_is_valid(void);

/**
 * @brief  Update frequency measurement (call periodically)
 *         Handles timeout detection for low/zero frequency
 */
void frequency_counter_update(void);

#ifdef __cplusplus
}
#endif

#endif /* __FREQUENCY_COUNTER_H */
