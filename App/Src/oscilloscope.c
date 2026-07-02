/**
 ******************************************************************************
 * @file    oscilloscope.c
 * @brief   示波器主控制器实现
 *
 *          协调 ADC 采样、波形处理、触发检测、FFT 分析和显示更新
 *          实现自动/正常/单次三种触发模式
 *          支持时域波形显示和频域 FFT 显示
 ******************************************************************************
 */

#include "oscilloscope.h"
#include "bsp_adc.h"
#include "ili9341.h"
#include "lcd_graphic.h"
#include "waveform.h"
#include "fft_analyzer.h"
#include "ui_menu.h"
#include "frequency_counter.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* Global configuration */
OscilloscopeConfig_t g_osc_config;

/* Raw ADC data buffers (per channel) */
static uint16_t raw_ch1[ADC_SAMPLE_NUM];
static uint16_t raw_ch2[ADC_SAMPLE_NUM];

/* Processed display data */
static uint16_t display_ch1[LCD_WAVEFORM_W];
static uint16_t display_ch2[LCD_WAVEFORM_W];

/* Previous waveform for local refresh (erase old, draw new) */
static uint16_t prev_ch1[LCD_WAVEFORM_W];
static uint16_t prev_ch2[LCD_WAVEFORM_W];
static uint8_t has_prev_waveform = 0;

/* Sample data after trigger */
static uint16_t triggered_ch1[ADC_SAMPLE_NUM];
static uint16_t triggered_ch2[ADC_SAMPLE_NUM];
static uint16_t triggered_count = 0;

/* Voltage divider ratio (hardware: 1:10 probe, or 1:1 direct) */
#define PROBE_RATIO  1.0f

/* Timebase table (seconds per division) corresponding to sample rates */
static const float timebase_table[SAMPLE_RATE_MAX] = {
    0.000001f,   /* 1 Msps:   1us/div (10 samples/div) */
    0.000002f,   /* 500 Ksps: 2us/div */
    0.000005f,   /* 200 Ksps: 5us/div */
    0.00001f,    /* 100 Ksps: 10us/div */
    0.00002f,    /* 50 Ksps:  20us/div */
    0.0001f,     /* 10 Ksps:  100us/div */
    0.001f       /* 1 Ksps:   1ms/div */
};

void Oscilloscope_Init(void)
{
    /* Initialize configuration with defaults */
    g_osc_config.sample_rate = SAMPLE_RATE_1M;
    g_osc_config.timebase = timebase_table[SAMPLE_RATE_1M];
    g_osc_config.trigger_level = 1.65f;   /* Half of VREF */
    g_osc_config.trigger_mode = TRIG_AUTO;
    g_osc_config.trigger_edge = TRIG_EDGE_RISING;
    g_osc_config.trigger_channel = 0;

    g_osc_config.ch1.enabled = 1;
    g_osc_config.ch1.coupling = 0;        /* DC */
    g_osc_config.ch1.voltage_div = 1.0f;
    g_osc_config.ch1.offset = 0;
    g_osc_config.ch1.color = COLOR_YELLOW;
    g_osc_config.ch1.adc_offset = 2048;   /* Mid-scale for 12-bit */

    g_osc_config.ch2.enabled = 1;
    g_osc_config.ch2.coupling = 0;
    g_osc_config.ch2.voltage_div = 1.0f;
    g_osc_config.ch2.offset = 0;
    g_osc_config.ch2.color = COLOR_CYAN;
    g_osc_config.ch2.adc_offset = 2048;

    g_osc_config.display_mode = DISP_WAVEFORM;
    g_osc_config.fft_channel = 0;

    g_osc_config.freq_hz = 0;
    g_osc_config.vpp_ch1 = 0;
    g_osc_config.vpp_ch2 = 0;
    g_osc_config.vavg_ch1 = 0;
    g_osc_config.vavg_ch2 = 0;
    g_osc_config.thd = 0;
    g_osc_config.fft_peak_freq = 0;
    g_osc_config.fft_peak_mag = 0;

    g_osc_config.triggered = 0;
    g_osc_config.single_triggered = 0;
    g_osc_config.last_display_time = 0;

    has_prev_waveform = 0;

    /* Set initial sample rate */
    bsp_adc_set_sample_rate(g_osc_config.sample_rate);
}

float Oscilloscope_ADC_to_Voltage(uint16_t adc_val, uint8_t ch)
{
    float voltage;
    ChannelConfig_t *cfg = (ch == 0) ? &g_osc_config.ch1 : &g_osc_config.ch2;

    voltage = (float)adc_val * ADC_VREF / (float)(ADC_MAX_VALUE + 1);
    voltage *= PROBE_RATIO;

    /* Apply AC coupling offset removal */
    if (cfg->coupling == 1)
    {
        voltage -= (float)cfg->adc_offset * ADC_VREF / (float)(ADC_MAX_VALUE + 1);
    }

    return voltage;
}

/**
 * @brief  Find trigger point in the sampled data
 * @param  data: Array of ADC values from trigger channel
 * @param  count: Number of samples
 * @return Index of trigger point, or -1 if not found
 */
static int32_t find_trigger(uint16_t *data, uint16_t count)
{
    uint16_t trigger_adc;
    uint32_t i;

    /* Convert trigger level to ADC value */
    trigger_adc = (uint16_t)(g_osc_config.trigger_level * (ADC_MAX_VALUE + 1) / ADC_VREF);
    if (trigger_adc > ADC_MAX_VALUE)
        trigger_adc = ADC_MAX_VALUE;

    /* Search for trigger point (skip first 10% for stability) */
    uint16_t start = count / 10;

    if (g_osc_config.trigger_edge == TRIG_EDGE_RISING)
    {
        for (i = start; i < count - 1; i++)
        {
            if (data[i] < trigger_adc && data[i + 1] >= trigger_adc)
            {
                return (int32_t)i;
            }
        }
    }
    else  /* Falling edge */
    {
        for (i = start; i < count - 1; i++)
        {
            if (data[i] > trigger_adc && data[i + 1] <= trigger_adc)
            {
                return (int32_t)i;
            }
        }
    }

    return -1;  /* No trigger found */
}

/**
 * @brief  Calculate Vpp, Vavg from ADC data
 */
static void calculate_measurements(uint16_t *data, uint16_t count, uint8_t ch)
{
    uint16_t min_val = 0xFFFF;
    uint16_t max_val = 0;
    uint32_t sum = 0;
    uint32_t i;

    for (i = 0; i < count; i++)
    {
        if (data[i] < min_val) min_val = data[i];
        if (data[i] > max_val) max_val = data[i];
        sum += data[i];
    }

    float vpp = Oscilloscope_ADC_to_Voltage(max_val, ch) -
                Oscilloscope_ADC_to_Voltage(min_val, ch);
    float vavg = Oscilloscope_ADC_to_Voltage(sum / count, ch);

    if (ch == 0)
    {
        g_osc_config.vpp_ch1 = vpp;
        g_osc_config.vavg_ch1 = vavg;
    }
    else
    {
        g_osc_config.vpp_ch2 = vpp;
        g_osc_config.vavg_ch2 = vavg;
    }
}

void Oscilloscope_Process(void)
{
    ADC_BufferStatus_t status;
    int32_t trig_index;

    /* Check if new ADC data is ready */
    status = bsp_adc_get_status();
    if (status == ADC_BUF_EMPTY)
    {
        /* Even without ADC data, update frequency counter timeout check */
        frequency_counter_update();
        return;
    }

    /* Get the data (de-interleaved into ch1/ch2 arrays) */
    if (bsp_adc_get_data(raw_ch1, raw_ch2) != BSP_OK)
    {
        return;
    }

    /* For single trigger mode, check if already triggered */
    if (g_osc_config.trigger_mode == TRIG_SINGLE && g_osc_config.single_triggered)
    {
        return;
    }

    /* Find trigger point */
    uint16_t *trig_data = (g_osc_config.trigger_channel == 0) ? raw_ch1 : raw_ch2;
    uint16_t sample_count = ADC_BUF_HALF_SIZE / ADC_CHANNELS;  /* 512 */

    trig_index = find_trigger(trig_data, sample_count);

    if (trig_index >= 0)
    {
        /* Trigger found - copy data starting from trigger point */
        g_osc_config.triggered = 1;
        uint16_t remaining = sample_count - trig_index;

        for (uint16_t i = 0; i < remaining; i++)
        {
            triggered_ch1[i] = raw_ch1[trig_index + i];
            triggered_ch2[i] = raw_ch2[trig_index + i];
        }
        triggered_count = remaining;

        if (g_osc_config.trigger_mode == TRIG_SINGLE)
        {
            g_osc_config.single_triggered = 1;
        }
    }
    else
    {
        /* No trigger found */
        g_osc_config.triggered = 0;

        if (g_osc_config.trigger_mode == TRIG_AUTO)
        {
            /* In auto mode, display untriggered data */
            for (uint16_t i = 0; i < sample_count; i++)
            {
                triggered_ch1[i] = raw_ch1[i];
                triggered_ch2[i] = raw_ch2[i];
            }
            triggered_count = sample_count;
        }
        else
        {
            /* Normal/Single mode: don't update display */
            return;
        }
    }

    /* Calculate measurements */
    calculate_measurements(triggered_ch1, triggered_count, 0);
    calculate_measurements(triggered_ch2, triggered_count, 1);

    /* Process FFT if in FFT or dual mode */
    if (g_osc_config.display_mode == DISP_FFT || g_osc_config.display_mode == DISP_DUAL)
    {
        uint16_t *fft_data = (g_osc_config.fft_channel == 0) ? triggered_ch1 : triggered_ch2;
        FFT_Analyze(fft_data, triggered_count, bsp_adc_get_sample_rate_hz());
    }
}

/**
 * @brief  Map ADC value to screen Y coordinate
 */
static uint16_t adc_to_screen_y(uint16_t adc_val, ChannelConfig_t *cfg)
{
    /* Map ADC value (0-4095) to screen Y (0 to waveform_height-1)
     * Center at waveform_height/2, scaled by voltage_div
   */
    float normalized = (float)(adc_val - cfg->adc_offset) / (float)ADC_MAX_VALUE;
    float volts = normalized * ADC_VREF * PROBE_RATIO;

    /* Scale by voltage per division (each div = 40 pixels for 200px height / 5 divs) */
    float pixels_per_volt = 40.0f / cfg->voltage_div;
    int16_t y_offset = (int16_t)(volts * pixels_per_volt);

    int16_t y = LCD_WAVEFORM_H / 2 - y_offset + cfg->offset;

    /* Clip to waveform area */
    if (y < 0) y = 0;
    if (y >= LCD_WAVEFORM_H) y = LCD_WAVEFORM_H - 1;

    return (uint16_t)y;
}

/**
 * @brief  Draw waveform on screen with local refresh
 */
static void draw_waveform(void)
{
    uint16_t i;
    uint16_t x_step;

    /* Calculate how many samples to display (fit to screen width) */
    uint16_t display_samples = triggered_count;
    if (display_samples > LCD_WAVEFORM_W)
    {
        display_samples = LCD_WAVEFORM_W;
    }

    /* Erase previous waveforms by drawing over them with background color */
    if (has_prev_waveform)
    {
        if (g_osc_config.ch1.enabled)
        {
            for (i = 0; i < LCD_WAVEFORM_W - 1; i++)
            {
                LCD_DrawPixel(i, prev_ch1[i], COLOR_BLACK);
                LCD_DrawPixel(i + 1, prev_ch1[i + 1], COLOR_BLACK);
            }
        }
        if (g_osc_config.ch2.enabled)
        {
            for (i = 0; i < LCD_WAVEFORM_W - 1; i++)
            {
                LCD_DrawPixel(i, prev_ch2[i], COLOR_BLACK);
                LCD_DrawPixel(i + 1, prev_ch2[i + 1], COLOR_BLACK);
            }
        }
    }

    /* Redraw grid if needed (after erasing) */
    LCD_DrawGrid(LCD_WAVEFORM_X, LCD_WAVEFORM_Y, LCD_WAVEFORM_W, LCD_WAVEFORM_H,
                 8, 6, COLOR_DARKGRAY);

    /* Draw new waveforms */
    x_step = (display_samples > 1) ? (display_samples - 1) / (LCD_WAVEFORM_W - 1) : 1;
    if (x_step == 0) x_step = 1;

    if (g_osc_config.ch1.enabled)
    {
        for (i = 0; i < LCD_WAVEFORM_W; i++)
        {
            uint16_t sample_idx = i * x_step;
            if (sample_idx >= display_samples)
                sample_idx = display_samples - 1;
            display_ch1[i] = adc_to_screen_y(triggered_ch1[sample_idx], &g_osc_config.ch1);
        }

        /* Draw waveform as connected line segments */
        for (i = 0; i < LCD_WAVEFORM_W - 1; i++)
        {
            LCD_DrawLine(i, display_ch1[i], i + 1, display_ch1[i + 1], g_osc_config.ch1.color);
        }

        /* Store for next frame erase */
        memcpy(prev_ch1, display_ch1, sizeof(prev_ch1));
    }

    if (g_osc_config.ch2.enabled)
    {
        for (i = 0; i < LCD_WAVEFORM_W; i++)
        {
            uint16_t sample_idx = i * x_step;
            if (sample_idx >= display_samples)
                sample_idx = display_samples - 1;
            display_ch2[i] = adc_to_screen_y(triggered_ch2[sample_idx], &g_osc_config.ch2);
        }

        for (i = 0; i < LCD_WAVEFORM_W - 1; i++)
        {
            LCD_DrawLine(i, display_ch2[i], i + 1, display_ch2[i + 1], g_osc_config.ch2.color);
        }

        memcpy(prev_ch2, display_ch2, sizeof(prev_ch2));
    }

    has_prev_waveform = 1;
}

/**
 * @brief  Draw FFT spectrum
 */
static void draw_fft(void)
{
    uint16_t i;
    float *mag = FFT_GetMagnitude();
    uint16_t fft_bins = FFT_LENGTH / 2;
    float max_mag = FFT_GetMaxMagnitude();

    if (max_mag <= 0)
    {
        return;
    }

    /* Clear waveform area */
    LCD_FillRect(LCD_WAVEFORM_X, LCD_WAVEFORM_Y, LCD_WAVEFORM_W, LCD_WAVEFORM_H, COLOR_BLACK);

    /* Draw FFT grid */
    LCD_DrawGrid(LCD_WAVEFORM_X, LCD_WAVEFORM_Y, LCD_WAVEFORM_W, LCD_WAVEFORM_H,
                 8, 6, COLOR_DARKGRAY);

    /* Draw FFT bars - map bins to screen width */
    uint16_t bar_width = LCD_WAVEFORM_W / 64;  /* Show 64 frequency bins */
    if (bar_width == 0) bar_width = 1;

    float scale = (float)(LCD_WAVEFORM_H - 10) / max_mag;

    for (i = 0; i < 64; i++)
    {
        uint16_t bin_idx = i * fft_bins / 64;
        if (bin_idx >= fft_bins) bin_idx = fft_bins - 1;

        uint16_t bar_height = (uint16_t)(mag[bin_idx] * scale);
        if (bar_height > LCD_WAVEFORM_H - 10)
            bar_height = LCD_WAVEFORM_H - 10;

        uint16_t x = i * bar_width;
        uint16_t y = LCD_WAVEFORM_H - bar_height - 1;

        /* Color: green for normal, red for peak */
        uint16_t color = (bin_idx == FFT_GetPeakBin()) ? COLOR_RED : COLOR_GREEN;
        LCD_FillRect(x, y, bar_width, bar_height, color);
    }

    /* Draw FFT info */
    char buf[32];
    snprintf(buf, sizeof(buf), "Peak: %.1fHz", g_osc_config.fft_peak_freq);
    LCD_DrawString(2, 2, buf, FONT_SMALL, COLOR_WHITE, COLOR_BLACK);
    snprintf(buf, sizeof(buf), "THD: %.1f%%", g_osc_config.thd);
    LCD_DrawString(2, 18, buf, FONT_SMALL, COLOR_YELLOW, COLOR_BLACK);
}

/**
 * @brief  Draw info panel (right side of screen)
 */
static void draw_info_panel(void)
{
    char buf[32];

    /* Clear info panel */
    LCD_FillRect(LCD_INFO_X, LCD_INFO_Y, LCD_INFO_W, LCD_INFO_H, COLOR_NAVY);

    /* Draw border */
    LCD_DrawVLine(LCD_INFO_X, LCD_INFO_Y, LCD_INFO_H, COLOR_GRAY);

    /* Channel 1 info */
    LCD_DrawString(LCD_INFO_X + 4, 4, "CH1", FONT_SMALL, COLOR_YELLOW, COLOR_NAVY);
    snprintf(buf, sizeof(buf), "%.2fV", g_osc_config.vpp_ch1);
    LCD_DrawString(LCD_INFO_X + 4, 20, buf, FONT_SMALL, COLOR_WHITE, COLOR_NAVY);
    snprintf(buf, sizeof(buf), "%.2fv", g_osc_config.vavg_ch1);
    LCD_DrawString(LCD_INFO_X + 4, 34, buf, FONT_SMALL, COLOR_GRAY, COLOR_NAVY);

    /* Channel 2 info */
    LCD_DrawString(LCD_INFO_X + 4, 54, "CH2", FONT_SMALL, COLOR_CYAN, COLOR_NAVY);
    snprintf(buf, sizeof(buf), "%.2fV", g_osc_config.vpp_ch2);
    LCD_DrawString(LCD_INFO_X + 4, 70, buf, FONT_SMALL, COLOR_WHITE, COLOR_NAVY);
    snprintf(buf, sizeof(buf), "%.2fv", g_osc_config.vavg_ch2);
    LCD_DrawString(LCD_INFO_X + 4, 84, buf, FONT_SMALL, COLOR_GRAY, COLOR_NAVY);

    /* Frequency */
    LCD_DrawString(LCD_INFO_X + 4, 104, "FRQ", FONT_SMALL, COLOR_ORANGE, COLOR_NAVY);
    if (g_osc_config.freq_hz > 1000)
    {
        snprintf(buf, sizeof(buf), "%.1fk", g_osc_config.freq_hz / 1000.0f);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.0f", g_osc_config.freq_hz);
    }
    LCD_DrawString(LCD_INFO_X + 4, 120, buf, FONT_SMALL, COLOR_WHITE, COLOR_NAVY);

    /* Timebase */
    LCD_DrawString(LCD_INFO_X + 4, 140, "T/D", FONT_SMALL, COLOR_ORANGE, COLOR_NAVY);
    if (g_osc_config.timebase < 0.001f)
    {
        snprintf(buf, sizeof(buf), "%.0fu", g_osc_config.timebase * 1e6f);
    }
    else if (g_osc_config.timebase < 1.0f)
    {
        snprintf(buf, sizeof(buf), "%.0fm", g_osc_config.timebase * 1e3f);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.1f", g_osc_config.timebase);
    }
    LCD_DrawString(LCD_INFO_X + 4, 156, buf, FONT_SMALL, COLOR_WHITE, COLOR_NAVY);

    /* Trigger mode */
    const char *trig_str[] = {"AUTO", "NORM", "SGL"};
    LCD_DrawString(LCD_INFO_X + 4, 176, trig_str[g_osc_config.trigger_mode],
                   FONT_SMALL, COLOR_MAGENTA, COLOR_NAVY);

    /* Trigger indicator */
    if (g_osc_config.triggered)
    {
        LCD_DrawString(LCD_INFO_X + 4, 192, "TRIG", FONT_SMALL, COLOR_GREEN, COLOR_NAVY);
    }
    else
    {
        LCD_DrawString(LCD_INFO_X + 4, 192, "----", FONT_SMALL, COLOR_DARKGRAY, COLOR_NAVY);
    }

    /* FFT info */
    if (g_osc_config.display_mode == DISP_FFT || g_osc_config.display_mode == DISP_DUAL)
    {
        snprintf(buf, sizeof(buf), "THD%.0f%%", g_osc_config.thd);
        LCD_DrawString(LCD_INFO_X + 4, 210, buf, FONT_SMALL, COLOR_RED, COLOR_NAVY);
    }
}

void Oscilloscope_Display(void)
{
    uint32_t now = HAL_GetTick();

    /* Rate limit to target FPS */
    if (now - g_osc_config.last_display_time < DISPLAY_INTERVAL_MS)
    {
        return;
    }
    g_osc_config.last_display_time = now;

    /* Draw based on display mode */
    switch (g_osc_config.display_mode)
    {
    case DISP_WAVEFORM:
        draw_waveform();
        break;
    case DISP_FFT:
        draw_fft();
        break;
    case DISP_DUAL:
        /* Split: top half waveform, bottom half FFT */
        draw_waveform();
        /* Could draw mini-FFT on bottom, simplified here */
        break;
    }

    /* Always update info panel */
    draw_info_panel();
}

OscilloscopeConfig_t *Oscilloscope_GetConfig(void)
{
    return &g_osc_config;
}

void Oscilloscope_ResetTrigger(void)
{
    g_osc_config.single_triggered = 0;
    g_osc_config.triggered = 0;
    has_prev_waveform = 0;
}
