/**
 ******************************************************************************
 * @file    ui_state_machine.h
 * @brief   UI 状态机 - 管理多级菜单导航与用户交互
 *
 *          状态层次：
 *          STATE_NORMAL (正常显示) 
 *            → [编码器按下] → STATE_MENU (菜单浏览)
 *              → [选择] → STATE_ADJUST (参数调整) 或 进入子菜单
 *            → [KEY1] → 切换显示模式
 *            → [KEY2] → 切换触发模式
 *            → [KEY3] → 切换 FFT/波形
 ******************************************************************************
 */

#ifndef __UI_STATE_MACHINE_H
#define __UI_STATE_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* UI states */
typedef enum {
    UI_STATE_NORMAL = 0,    /* 正常显示模式 - 显示波形/FFT */
    UI_STATE_MENU = 1,      /* 菜单浏览模式 */
    UI_STATE_ADJUST = 2,    /* 参数调整模式 */
    UI_STATE_INFO = 3       /* 信息显示模式 */
} UI_State_t;

/**
 * @brief  Initialize UI state machine
 */
void UI_StateMachine_Init(void);

/**
 * @brief  Process UI state machine (called in main loop)
 *         Handles encoder and button input, transitions states
 */
void UI_StateMachine_Process(void);

/**
 * @brief  Get current UI state
 * @return Current state
 */
UI_State_t UI_StateMachine_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_STATE_MACHINE_H */
