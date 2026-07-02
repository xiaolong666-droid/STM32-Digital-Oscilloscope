/**
 ******************************************************************************
 * @file    bsp_timer.h
 * @brief   Timer utility functions (debounce timer manager)
 ******************************************************************************
 */

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/**
 * @brief  Initialize timer utilities (start debounce timer)
 */
void bsp_timer_init(void);

/**
 * @brief  Get milliseconds since boot (wraps at 49 days)
 * @return Millisecond timestamp
 */
uint32_t bsp_timer_get_ms(void);

/**
 * @brief  Check if a timeout has elapsed
 * @param  start: Start timestamp
 * @param  timeout_ms: Timeout duration
 * @return 1 if elapsed, 0 otherwise
 */
uint8_t bsp_timer_elapsed(uint32_t start, uint32_t timeout_ms);

/**
 * @brief  Debounce timer callback (called from TIM6 ISR at 10ms)
 */
void bsp_timer_debounce_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TIMER_H */
