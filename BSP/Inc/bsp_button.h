/**
 ******************************************************************************
 * @file    bsp_button.h
 * @brief   User button driver (3 buttons + debounce)
 ******************************************************************************
 */

#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* Button identifiers */
typedef enum {
    BUTTON_KEY1 = 0,    /* PC13 */
    BUTTON_KEY2 = 1,    /* PC14 */
    BUTTON_KEY3 = 2,    /* PC15 */
    BUTTON_MAX
} ButtonID_t;

/* Button events */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_CLICKED,    /* Press + release */
    BUTTON_EVENT_LONG_PRESS  /* Held > 1 second */
} ButtonEvent_t;

/**
 * @brief  Initialize button driver
 */
void bsp_button_init(void);

/**
 * @brief  Get button event (non-blocking)
 * @param  id: Button ID
 * @return ButtonEvent_t
 */
ButtonEvent_t bsp_button_get_event(ButtonID_t id);

/**
 * @brief  Check if button is currently pressed (debounced)
 * @param  id: Button ID
 * @return 1=pressed, 0=released
 */
uint8_t bsp_button_is_pressed(ButtonID_t id);

/**
 * @brief  Button EXTI callback (called from ISR)
 * @param  id: Button ID that triggered
 */
void bsp_button_callback(ButtonID_t id);

/**
 * @brief  Button debounce process (called from TIM6 ISR at 10ms)
 */
void bsp_button_debounce_process(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_BUTTON_H */
