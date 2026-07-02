/**
 ******************************************************************************
 * @file    ui_menu.c
 * @brief   UI 菜单系统实现
 *          基于状态机实现多级菜单，支持编码器旋钮与按键双模式操作
 ******************************************************************************
 */

#include "ui_menu.h"
#include "lcd_graphic.h"
#include "oscilloscope.h"
#include "bsp_adc.h"
#include <string.h>
#include <stdio.h>

/* Current menu state */
static MenuID_t current_menu = MENU_NONE;
static int8_t menu_cursor = 0;     /* Selected item index */
static uint8_t menu_visible = 0;   /* Is menu currently displayed? */

/* Menu item counts */
#define MAIN_MENU_ITEMS    6
#define TIMEBASE_ITEMS     7
#define VOLTAGE_ITEMS      6
#define TRIGGER_ITEMS      5
#define FFT_ITEMS          4
#define SYSTEM_ITEMS       4

/* Main menu items */
static const MenuItem_t main_menu[MAIN_MENU_ITEMS] = {
    {"1.Timebase",     MENU_TIMEBASE, NULL},
    {"2.Voltage",      MENU_VOLTAGE,  NULL},
    {"3.Trigger",      MENU_TRIGGER,  NULL},
    {"4.FFT Analysis", MENU_FFT,      NULL},
    {"5.System",       MENU_SYSTEM,   NULL},
    {"6.Info",         MENU_INFO,     NULL},
};

/* Timebase menu items */
static const MenuItem_t timebase_menu[TIMEBASE_ITEMS] = {
    {"1 Msps  1us/div",  MENU_NONE, NULL},
    {"500Ksps 2us/div",  MENU_NONE, NULL},
    {"200Ksps 5us/div",  MENU_NONE, NULL},
    {"100Ksps 10us/div", MENU_NONE, NULL},
    {"50Ksps  20us/div", MENU_NONE, NULL},
    {"10Ksps  100us/div",MENU_NONE, NULL},
    {"1Ksps   1ms/div",  MENU_NONE, NULL},
};

/* Voltage per division menu */
static const MenuItem_t voltage_menu[VOLTAGE_ITEMS] = {
    {"0.1V/div", MENU_NONE, NULL},
    {"0.2V/div", MENU_NONE, NULL},
    {"0.5V/div", MENU_NONE, NULL},
    {"1.0V/div", MENU_NONE, NULL},
    {"2.0V/div", MENU_NONE, NULL},
    {"5.0V/div", MENU_NONE, NULL},
};

/* Trigger settings menu */
static const MenuItem_t trigger_menu[TRIGGER_ITEMS] = {
    {"Auto",     MENU_NONE, NULL},
    {"Normal",   MENU_NONE, NULL},
    {"Single",   MENU_NONE, NULL},
    {"Rising",   MENU_NONE, NULL},
    {"Falling",  MENU_NONE, NULL},
};

/* FFT settings menu */
static const MenuItem_t fft_menu[FFT_ITEMS] = {
    {"FFT Mode",     MENU_NONE, NULL},
    {"Waveform Mode",MENU_NONE, NULL},
    {"Dual Mode",    MENU_NONE, NULL},
    {"Hanning Win",  MENU_NONE, NULL},
};

/* System menu */
static const MenuItem_t system_menu[SYSTEM_ITEMS] = {
    {"CH1 On/Off",   MENU_NONE, NULL},
    {"CH2 On/Off",   MENU_NONE, NULL},
    {"Reset Trig",   MENU_NONE, NULL},
    {"Back",         MENU_MAIN, NULL},
};

/* Helper: get menu items and count for a given menu ID */
static const MenuItem_t *get_menu_items(MenuID_t id, uint8_t *count)
{
    switch (id)
    {
    case MENU_MAIN:     *count = MAIN_MENU_ITEMS;   return main_menu;
    case MENU_TIMEBASE: *count = TIMEBASE_ITEMS;    return timebase_menu;
    case MENU_VOLTAGE:  *count = VOLTAGE_ITEMS;     return voltage_menu;
    case MENU_TRIGGER:  *count = TRIGGER_ITEMS;     return trigger_menu;
    case MENU_FFT:      *count = FFT_ITEMS;         return fft_menu;
    case MENU_SYSTEM:   *count = SYSTEM_ITEMS;      return system_menu;
    default:            *count = 0;                 return NULL;
    }
}

void UI_Menu_Init(void)
{
    current_menu = MENU_NONE;
    menu_cursor = 0;
    menu_visible = 0;
}

MenuID_t UI_Menu_GetCurrent(void)
{
    return current_menu;
}

void UI_Menu_Enter(MenuID_t id)
{
    current_menu = id;
    menu_cursor = 0;
    menu_visible = 1;
}

void UI_Menu_Back(void)
{
    if (current_menu == MENU_MAIN || current_menu == MENU_NONE)
    {
        /* Exit menu */
        current_menu = MENU_NONE;
        menu_visible = 0;
    }
    else
    {
        current_menu = MENU_MAIN;
        menu_cursor = 0;
    }
}

void UI_Menu_Navigate(int8_t direction)
{
    uint8_t count;
    const MenuItem_t *items = get_menu_items(current_menu, &count);

    if (count == 0)
        return;

    if (direction > 0)
    {
        menu_cursor++;
        if (menu_cursor >= count)
            menu_cursor = 0;
    }
    else
    {
        menu_cursor--;
        if (menu_cursor < 0)
            menu_cursor = count - 1;
    }
}

void UI_Menu_Select(void)
{
    uint8_t count;
    const MenuItem_t *items = get_menu_items(current_menu, &count);
    OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();

    if (count == 0 || menu_cursor >= count)
        return;

    const MenuItem_t *selected = &items[menu_cursor];

    /* Handle leaf actions based on current menu */
    switch (current_menu)
    {
    case MENU_MAIN:
        if (selected->sub_menu != MENU_NONE)
        {
            UI_Menu_Enter(selected->sub_menu);
        }
        break;

    case MENU_TIMEBASE:
        cfg->sample_rate = (SampleRate_t)menu_cursor;
        bsp_adc_set_sample_rate(cfg->sample_rate);
        UI_Menu_Back();
        break;

    case MENU_VOLTAGE:
        {
            float voltages[] = {0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 5.0f};
            cfg->ch1.voltage_div = voltages[menu_cursor];
            cfg->ch2.voltage_div = voltages[menu_cursor];
        }
        UI_Menu_Back();
        break;

    case MENU_TRIGGER:
        if (menu_cursor < 3)
        {
            cfg->trigger_mode = (TriggerMode_t)menu_cursor;
        }
        else
        {
            cfg->trigger_edge = (TriggerEdge_t)(menu_cursor - 3);
        }
        if (cfg->trigger_mode == TRIG_SINGLE)
        {
            Oscilloscope_ResetTrigger();
        }
        UI_Menu_Back();
        break;

    case MENU_FFT:
        if (menu_cursor < 3)
        {
            cfg->display_mode = (DisplayMode_t)menu_cursor;
        }
        UI_Menu_Back();
        break;

    case MENU_SYSTEM:
        switch (menu_cursor)
        {
        case 0: cfg->ch1.enabled = !cfg->ch1.enabled; break;
        case 1: cfg->ch2.enabled = !cfg->ch2.enabled; break;
        case 2: Oscilloscope_ResetTrigger(); break;
        case 3: UI_Menu_Back(); break;
        }
        break;

    case MENU_INFO:
        UI_Menu_Back();
        break;

    default:
        break;
    }
}

void UI_Menu_Draw(void)
{
    if (!menu_visible || current_menu == MENU_NONE)
    {
        return;
    }

    uint8_t count, i;
    const MenuItem_t *items = get_menu_items(current_menu, &count);

    if (count == 0)
        return;

    /* Draw menu overlay (semi-transparent style: dark background) */
    LCD_FillRect(20, 30, LCD_WAVEFORM_W - 40, LCD_WAVEFORM_H - 60, COLOR_NAVY);
    LCD_DrawRect(20, 30, LCD_WAVEFORM_W - 40, LCD_WAVEFORM_H - 60, COLOR_WHITE);

    /* Draw title */
    const char *title = "";
    switch (current_menu)
    {
    case MENU_MAIN:     title = "MAIN MENU";     break;
    case MENU_TIMEBASE: title = "TIMEBASE";      break;
    case MENU_VOLTAGE:  title = "VOLTAGE/DIV";   break;
    case MENU_TRIGGER:  title = "TRIGGER";       break;
    case MENU_FFT:      title = "FFT SETTINGS";  break;
    case MENU_SYSTEM:   title = "SYSTEM";        break;
    case MENU_INFO:     title = "INFO";          break;
    default:            title = "MENU";          break;
    }

    LCD_DrawStringCenter(20, 34, LCD_WAVEFORM_W - 40, title, FONT_SMALL,
                         COLOR_YELLOW, COLOR_NAVY);

    /* Draw menu items */
    uint16_t y = 52;
    uint8_t max_visible = 8;
    uint8_t start = 0;

    if (menu_cursor >= max_visible)
    {
        start = menu_cursor - max_visible + 1;
    }

    for (i = 0; i < count && i < max_visible; i++)
    {
        uint8_t idx = start + i;
        if (idx >= count)
            break;

        uint16_t fg, bg;
        if (idx == menu_cursor)
        {
            fg = COLOR_BLACK;
            bg = COLOR_CYAN;
            LCD_FillRect(26, y, LCD_WAVEFORM_W - 52, 14, bg);
        }
        else
        {
            fg = COLOR_WHITE;
            bg = COLOR_NAVY;
        }

        LCD_DrawString(30, y + 1, items[idx].label, FONT_SMALL, fg, bg);
        y += 16;
    }

    /* Draw navigation hint */
    LCD_DrawString(30, LCD_WAVEFORM_H - 32, "Enc:Nav  Btn:Sel", FONT_SMALL,
                   COLOR_GRAY, COLOR_NAVY);
}

void UI_Menu_DrawStatusBar(void)
{
    char buf[40];
    OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();

    /* Clear status bar area */
    LCD_FillRect(LCD_MENU_X, LCD_MENU_Y, LCD_MENU_W, LCD_MENU_H, COLOR_DARKGRAY);
    LCD_DrawHLine(LCD_MENU_X, LCD_MENU_Y, LCD_MENU_W, COLOR_GRAY);

    /* Display mode indicator */
    const char *mode_str[] = {"WAVE", "FFT", "DUAL"};
    snprintf(buf, sizeof(buf), "[%s]", mode_str[cfg->display_mode]);
    LCD_DrawString(4, 206, buf, FONT_SMALL, COLOR_GREEN, COLOR_DARKGRAY);

    /* Sample rate */
    uint32_t sr = bsp_adc_get_sample_rate_hz();
    if (sr >= 1000000)
        snprintf(buf, sizeof(buf), "%luMsps", sr / 1000000);
    else if (sr >= 1000)
        snprintf(buf, sizeof(buf), "%luKsps", sr / 1000);
    else
        snprintf(buf, sizeof(buf), "%lusps", sr);
    LCD_DrawString(60, 206, buf, FONT_SMALL, COLOR_WHITE, COLOR_DARKGRAY);

    /* V/div */
    snprintf(buf, sizeof(buf), "%.1fV", cfg->ch1.voltage_div);
    LCD_DrawString(130, 206, buf, FONT_SMALL, COLOR_YELLOW, COLOR_DARKGRAY);

    /* Trigger mode */
    const char *trig_str[] = {"AUTO", "NORM", "SGL"};
    snprintf(buf, sizeof(buf), "T:%s", trig_str[cfg->trigger_mode]);
    LCD_DrawString(170, 206, buf, FONT_SMALL, COLOR_MAGENTA, COLOR_DARKGRAY);

    /* Menu hint */
    if (menu_visible)
    {
        LCD_DrawString(200, 206, "[MENU]", FONT_SMALL, COLOR_CYAN, COLOR_DARKGRAY);
    }
    else
    {
        LCD_DrawString(195, 206, "Enc:Menu", FONT_SMALL, COLOR_GRAY, COLOR_DARKGRAY);
    }
}
