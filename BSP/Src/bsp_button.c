/**
 ******************************************************************************
 * @file    bsp_button.c
 * @brief   用户按键驱动实现
 *
 *          3个用户按键 (PC13/PC14/PC15)，外部中断下降沿触发
 *          TIM6 定时器 10ms 消抖
 *          支持短按、长按检测
 ******************************************************************************
 */

#include "bsp_button.h"

/* Button pin mapping */
static const uint16_t button_pins[BUTTON_MAX] = {
    KEY1_PIN,   /* PC13 */
    KEY2_PIN,   /* PC14 */
    KEY3_PIN    /* PC15 */
};

/* Button state structure */
typedef struct {
    uint8_t raw_state;          /* Raw pin state (active low: 0=pressed) */
    uint8_t debounced;          /* Debounced state */
    uint8_t debounce_counter;   /* Debounce counter */
    uint8_t int_triggered;      /* EXTI trigger flag */
    uint16_t press_duration;    /* Press duration counter (10ms units) */
    uint8_t clicked_flag;       /* Click event flag */
    uint8_t long_press_flag;    /* Long press event flag */
    uint8_t was_pressed;        /* Previous debounced state for edge detection */
} ButtonState_t;

static ButtonState_t button_states[BUTTON_MAX];

void bsp_button_init(void)
{
    uint8_t i;
    for (i = 0; i < BUTTON_MAX; i++)
    {
        button_states[i].raw_state = 1;        /* Not pressed (pull-up) */
        button_states[i].debounced = 1;
        button_states[i].debounce_counter = 0;
        button_states[i].int_triggered = 0;
        button_states[i].press_duration = 0;
        button_states[i].clicked_flag = 0;
        button_states[i].long_press_flag = 0;
        button_states[i].was_pressed = 1;
    }
}

ButtonEvent_t bsp_button_get_event(ButtonID_t id)
{
    if (id >= BUTTON_MAX)
    {
        return BUTTON_EVENT_NONE;
    }

    ButtonState_t *btn = &button_states[id];

    /* Check long press first */
    if (btn->long_press_flag)
    {
        btn->long_press_flag = 0;
        return BUTTON_EVENT_LONG_PRESS;
    }

    /* Check click */
    if (btn->clicked_flag)
    {
        btn->clicked_flag = 0;
        return BUTTON_EVENT_CLICKED;
    }

    return BUTTON_EVENT_NONE;
}

uint8_t bsp_button_is_pressed(ButtonID_t id)
{
    if (id >= BUTTON_MAX)
    {
        return 0;
    }
    return (button_states[id].debounced == 0) ? 1 : 0;
}

void bsp_button_callback(ButtonID_t id)
{
    if (id < BUTTON_MAX)
    {
        button_states[id].int_triggered = 1;
    }
}

void bsp_button_debounce_process(void)
{
    uint8_t i;
    uint8_t current_pin;

    for (i = 0; i < BUTTON_MAX; i++)
    {
        ButtonState_t *btn = &button_states[i];

        if (!btn->int_triggered)
        {
            /* Even without interrupt, track press duration */
            if (btn->debounced == 0)  /* Currently pressed */
            {
                btn->press_duration++;
                if (btn->press_duration >= 100 && !btn->long_press_flag)
                {
                    /* 100 * 10ms = 1 second long press */
                    btn->long_press_flag = 1;
                }
            }
            continue;
        }

        /* Read actual pin state */
        current_pin = HAL_GPIO_ReadPin(KEY_PORT, button_pins[i]);

        if (current_pin != btn->raw_state)
        {
            btn->raw_state = current_pin;
            btn->debounce_counter = 0;
            continue;
        }

        btn->debounce_counter++;

        /* After 3 consecutive matches (30ms), accept as stable */
        if (btn->debounce_counter >= 3)
        {
            btn->debounce_counter = 0;

            if (btn->raw_state != btn->debounced)
            {
                /* State changed */
                uint8_t was_pressed = (btn->debounced == 0);
                btn->debounced = btn->raw_state;

                if (btn->debounced == 0)
                {
                    /* Button pressed (falling edge) */
                    btn->press_duration = 0;
                }
                else
                {
                    /* Button released (rising edge) */
                    if (was_pressed && btn->press_duration < 100)
                    {
                        /* Short press = click */
                        btn->clicked_flag = 1;
                    }
                    btn->press_duration = 0;
                }
            }

            btn->int_triggered = 0;
        }
    }
}
