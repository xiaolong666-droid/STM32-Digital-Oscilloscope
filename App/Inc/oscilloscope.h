/**
 ******************************************************************************
 * @file    oscilloscope.h
 * @brief   示波器主控制器 - 协调采样、处理、显示
 ******************************************************************************
 */

#ifndef __OSCILLOSCOPE_H
#define __OSCILLOSCOPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "bsp_adc.h"

/* Trigger modes */
typedef enum {
    TRIG_AUTO = 0,      /* Auto trigger - always display */
    TRIG_NORMAL = 1,    /* Normal trigger - display only when triggered */
    TRIG_SINGLE = 2     /* Single trigger - display once then stop */
} TriggerMode_t;

/* Trigger edge */
typedef enum {
    TRIG_EDGE_RISING = 0,
    TRIG_EDGE_FALLING = 1
} TriggerEdge_t;

/* Display modes */
typedef enum {
    DISP_WAVEFORM = 0,  /* Time domain waveform */
    DISP_FFT = 1,       /* Frequency domain spectrum */
    DISP_DUAL = 2       /* Split screen: waveform + FFT */
} DisplayMode_t;

/* Channel configuration */
typedef struct {
    uint8_t enabled;            /* Channel on/off */
    uint8_t coupling;           /* 0=DC, 1=AC */
    float voltage_div;          /* Volts per division */
    int16_t offset;             /* Vertical offset in pixels */
    uint16_t color;             /* Waveform color */
    uint16_t adc_offset;        /* ADC zero offset (for AC coupling) */
} ChannelConfig_t;

/* Oscilloscope configuration */
typedef struct {
    /* Horizontal */
    SampleRate_t sample_rate;   /* Current sample rate */
    float timebase;             /* Time per division (seconds) */
    float trigger_level;        /* Trigger level in volts */
    TriggerMode_t trigger_mode;
    TriggerEdge_t trigger_edge;
    uint8_t trigger_channel;    /* 0=CH1, 1=CH2 */

    /* Vertical */
    ChannelConfig_t ch1;
    ChannelConfig_t ch2;

    /* Display */
    DisplayMode_t display_mode;
    uint8_t fft_channel;        /* Which channel to FFT (0 or 1) */

    /* Measurement results */
    float freq_hz;              /* Measured frequency */
    float vpp_ch1;              /* CH1 peak-to-peak voltage */
    float vpp_ch2;              /* CH2 peak-to-peak voltage */
    float vavg_ch1;             /* CH1 average voltage */
    float vavg_ch2;             /* CH2 average voltage */
    float thd;                  /* Total harmonic distortion (%) */
    float fft_peak_freq;        /* FFT peak frequency */
    float fft_peak_mag;         /* FFT peak magnitude */

    /* State */
    uint8_t triggered;          /* Trigger found flag */
    uint8_t single_triggered;   /* Single trigger already fired */
    uint32_t last_display_time; /* Last display update timestamp */
} OscilloscopeConfig_t;

/* Global oscilloscope config */
extern OscilloscopeConfig_t g_osc_config;

/**
 * @brief  Initialize oscilloscope
 */
void Oscilloscope_Init(void);

/**
 * @brief  Process incoming ADC data (called in main loop)
 *         - Reads DMA buffer
 *         - Applies trigger
 *         - Prepares display data
 */
void Oscilloscope_Process(void);

/**
 * @brief  Update display (called in main loop)
 *         - Draws waveform/FFT
 *         - Updates info panel
 *         - Rate limited to 30fps
 */
void Oscilloscope_Display(void);

/**
 * @brief  Get current oscilloscope configuration
 * @return Pointer to config struct
 */
OscilloscopeConfig_t *Oscilloscope_GetConfig(void);

/**
 * @brief  Convert ADC value to voltage
 * @param  adc_val: Raw ADC value (0-4095)
 * @param  ch: Channel index (0 or 1)
 * @return Voltage in volts
 */
float Oscilloscope_ADC_to_Voltage(uint16_t adc_val, uint8_t ch);

/**
 * @brief  Reset single trigger for next acquisition
 */
void Oscilloscope_ResetTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* __OSCILLOSCOPE_H */
