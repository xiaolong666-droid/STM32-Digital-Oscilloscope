/**
 ******************************************************************************
 * @file    ui_state_machine.c
 * @brief   UI 状态机实现
 *
 *          基于状态机管理多级菜单导航与用户交互
 *          支持编码器旋钮（外部中断 + 定时器消抖）与按键双模式操作
 *
 *          状态转换：
 *          STATE_NORMAL --[编码器按下]--> STATE_MENU
 *          STATE_MENU   --[编码器旋转]--> 导航菜单项
 *          STATE_MENU   --[编码器按下]--> 选择/进入子菜单
 *          STATE_MENU   --[KEY3/长按]--> 返回/退出
 *          STATE_NORMAL --[KEY1]--> 切换显示模式
 *          STATE_NORMAL --[KEY2]--> 切换触发模式
 *          STATE_NORMAL --[KEY3]--> 重置单次触发
 ******************************************************************************
 */

#include "ui_state_machine.h"
#include "ui_menu.h"
#include "oscilloscope.h"
#include "bsp_encoder.h"
#include "bsp_button.h"
#include "lcd_graphic.h"
#include <string.h>

/* Current state */
static UI_State_t ui_state = UI_STATE_NORMAL;

/* Track display mode for quick cycling */
static const DisplayMode_t display_modes[] = {DISP_WAVEFORM, DISP_FFT, DISP_DUAL};
static const uint8_t num_display_modes = 3;
static uint8_t current_display_idx = 0;

/* Track trigger mode for quick cycling */
static const TriggerMode_t trigger_modes[] = {TRIG_AUTO, TRIG_NORMAL, TRIG_SINGLE};
static const uint8_t num_trigger_modes = 3;
static uint8_t current_trigger_idx = 0;

void UI_StateMachine_Init(void)
{
    ui_state = UI_STATE_NORMAL;
    current_display_idx = 0;
    current_trigger_idx = 0;
    UI_Menu_Init();
}

/**
 * @brief  Handle input in NORMAL state
 */
static void process_normal_state(void)
{
    int16_t enc_delta;
    uint8_t enc_clicked;
    ButtonEvent_t btn_event;

    /* Check encoder rotation - adjust trigger level */
    enc_delta = bsp_encoder_get_delta();
    if (enc_delta != 0)
    {
        OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();
        /* Adjust trigger level: 0.1V per step */
        cfg->trigger_level += (float)enc_delta * 0.1f;
        if (cfg->trigger_level < 0) cfg->trigger_level = 0;
        if (cfg->trigger_level > ADC_VREF) cfg->trigger_level = ADC_VREF;
    }

    /* Check encoder button click - enter menu */
    enc_clicked = bsp_encoder_button_clicked();
    if (enc_clicked)
    {
        ui_state = UI_STATE_MENU;
        UI_Menu_Enter(MENU_MAIN);
        return;
    }

    /* Check user buttons */
    btn_event = bsp_button_get_event(BUTTON_KEY1);
    if (btn_event == BUTTON_EVENT_CLICKED)
    {
        /* KEY1: Cycle display mode */
        current_display_idx = (current_display_idx + 1) % num_display_modes;
        Oscilloscope_GetConfig()->display_mode = display_modes[current_display_idx];
    }

    btn_event = bsp_button_get_event(BUTTON_KEY2);
    if (btn_event == BUTTON_EVENT_CLICKED)
    {
        /* KEY2: Cycle trigger mode */
        current_trigger_idx = (current_trigger_idx + 1) % num_trigger_modes;
        OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();
        cfg->trigger_mode = trigger_modes[current_trigger_idx];
        if (cfg->trigger_mode == TRIG_SINGLE)
        {
            Oscilloscope_ResetTrigger();
        }
    }

    btn_event = bsp_button_get_event(BUTTON_KEY3);
    if (btn_event == BUTTON_EVENT_CLICKED)
    {
        /* KEY3: Reset single trigger */
        Oscilloscope_ResetTrigger();
    }

    /* Draw status bar in normal mode */
    UI_Menu_DrawStatusBar();
}

/**
 * @brief  Handle input in MENU state
 */
static void process_menu_state(void)
{
    int16_t enc_delta;
    uint8_t enc_clicked;
    ButtonEvent_t btn_event;

    /* Check encoder rotation - navigate menu */
    enc_delta = bsp_encoder_get_delta();
    if (enc_delta != 0)
    {
        UI_Menu_Navigate(enc_delta);
    }

    /* Check encoder button click - select menu item */
    enc_clicked = bsp_encoder_button_clicked();
    if (enc_clicked)
    {
        UI_Menu_Select();

        /* Check if we exited menu */
        if (UI_Menu_GetCurrent() == MENU_NONE)
        {
            ui_state = UI_STATE_NORMAL;
            return;
        }
    }

    /* Check KEY3 for back/exit */
    btn_event = bsp_button_get_event(BUTTON_KEY3);
    if (btn_event == BUTTON_EVENT_CLICKED)
    {
        UI_Menu_Back();
        if (UI_Menu_GetCurrent() == MENU_NONE)
        {
            ui_state = UI_STATE_NORMAL;
        }
    }

    /* Check KEY2 for back */
    btn_event = bsp_button_get_event(BUTTON_KEY2);
    if (btn_event == BUTTON_EVENT_CLICKED)
    {
        UI_Menu_Back();
        if (UI_Menu_GetCurrent() == MENU_NONE)
        {
            ui_state = UI_STATE_NORMAL;
        }
    }

    /* Draw menu overlay */
    UI_Menu_Draw();
}

/**
 * @brief  Handle input in ADJUST state (for parameter fine-tuning)
 */
static void process_adjust_state(void)
{
    int16_t enc_delta;
    uint8_t enc_clicked;

    /* Encoder rotation adjusts current parameter */
    enc_delta = bsp_encoder_get_delta();
    if (enc_delta != 0)
    {
        /* Parameter adjustment logic would go here */
        /* This is a placeholder for specific parameter adjustment */
    }

    /* Encoder click exits adjust mode */
    enc_clicked = bsp_encoder_button_clicked();
    if (enc_clicked)
    {
        ui_state = UI_STATE_MENU;
    }
}

void UI_StateMachine_Process(void)
{
    switch (ui_state)
    {
    case UI_STATE_NORMAL:
        process_normal_state();
        break;

    case UI_STATE_MENU:
        process_menu_state();
        break;

    case UI_STATE_ADJUST:
        process_adjust_state();
        break;

    case UI_STATE_INFO:
        /* Info display - any input returns to normal */
        if (bsp_encoder_button_clicked() ||
            bsp_button_get_event(BUTTON_KEY1) != BUTTON_EVENT_NONE ||
            bsp_button_get_event(BUTTON_KEY2) != BUTTON_EVENT_NONE ||
            bsp_button_get_event(BUTTON_KEY3) != BUTTON_EVENT_NONE)
        {
            ui_state = UI_STATE_NORMAL;
        }
        break;
    }
}

UI_State_t UI_StateMachine_GetState(void)
{
    return ui_state;
}
