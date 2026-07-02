/**
 ******************************************************************************
 * @file    ui_menu.h
 * @brief   UI 菜单系统 - 多级菜单定义与渲染
 ******************************************************************************
 */

#ifndef __UI_MENU_H
#define __UI_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "ili9341.h"

/* Menu IDs */
typedef enum {
    MENU_NONE = 0,
    MENU_MAIN,
    MENU_TIMEBASE,
    MENU_VOLTAGE,
    MENU_TRIGGER,
    MENU_FFT,
    MENU_SYSTEM,
    MENU_INFO
} MenuID_t;

/* Menu item structure */
typedef struct {
    const char *label;        /* Display text */
    MenuID_t sub_menu;        /* Sub-menu to enter (MENU_NONE = leaf) */
    void (*action)(void);     /* Action function (for leaf items) */
} MenuItem_t;

/**
 * @brief  Initialize menu system
 */
void UI_Menu_Init(void);

/**
 * @brief  Get current menu ID
 * @return Current menu ID
 */
MenuID_t UI_Menu_GetCurrent(void);

/**
 * @brief  Enter a specific menu
 * @param  id: Menu ID to enter
 */
void UI_Menu_Enter(MenuID_t id);

/**
 * @brief  Go back to parent menu
 */
void UI_Menu_Back(void);

/**
 * @brief  Navigate selection up/down
 * @param  direction: 1=down, -1=up
 */
void UI_Menu_Navigate(int8_t direction);

/**
 * @brief  Execute selected item
 */
void UI_Menu_Select(void);

/**
 * @brief  Draw the current menu on screen
 */
void UI_Menu_Draw(void);

/**
 * @brief  Draw the bottom status bar
 */
void UI_Menu_DrawStatusBar(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_MENU_H */
