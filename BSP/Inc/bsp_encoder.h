/**
 ******************************************************************************
 * @file    bsp_encoder.h
 * @brief   Rotary encoder driver (TIM4 encoder mode + EXTI button)
 ******************************************************************************
 */

#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* Encoder event types */
typedef enum {
    ENC_EVENT_NONE = 0,
    ENC_EVENT_CW,           /* Clockwise rotation (increase) */
    ENC_EVENT_CCW,          /* Counter-clockwise rotation (decrease) */
    ENC_EVENT_BUTTON_DOWN,  /* Button pressed */
    ENC_EVENT_BUTTON_UP     /* Button released */
} EncoderEvent_t;

/**
 * @brief  Initialize encoder driver
 */
void bsp_encoder_init(void);

/**
 * @brief  Get encoder rotation count (delta since last read)
 * @return Positive = CW steps, Negative = CCW steps
 */
int16_t bsp_encoder_get_delta(void);

/**
 * @brief  Get encoder absolute count
 * @return Current counter value
 */
int16_t bsp_encoder_get_count(void);

/**
 * @brief  Reset encoder count to zero
 */
void bsp_encoder_reset(void);

/**
 * @brief  Get encoder button state (debounced)
 * @return 1=pressed, 0=released
 */
uint8_t bsp_encoder_button_pressed(void);

/**
 * @brief  Check if encoder button was clicked (press+release)
 * @return 1 if clicked since last check, 0 otherwise
 */
uint8_t bsp_encoder_button_clicked(void);

/**
 * @brief  Encoder button EXTI callback (called from ISR)
 */
void bsp_encoder_btn_callback(void);

/**
 * @brief  Debounce processing (called from TIM6 ISR at 10ms)
 */
void bsp_encoder_debounce_process(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ENCODER_H */
