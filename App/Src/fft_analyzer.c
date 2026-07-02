/**
 ******************************************************************************
 * @file    fft_analyzer.c
 * @brief   FFT 频谱分析模块实现
 *
 *          移植 CMSIS-DSP 库，使用 1024 点复数 FFT
 *          支持窗函数（Hanning/Hamming/Blackman）、频率峰值检测、THD 计算
 *
 *          工作流程：
 *          1. 将 ADC 数据转为浮点数并施加窗函数
 *          2. 构造复数输入 (实部=数据，虚部=0)
 *          3. 调用 arm_cfft_f32 执行 FFT
 *          4. 计算幅度谱 arm_cmplx_mag_f32
 *          5. 检测峰值频率
 *          6. 计算 THD = sqrt(V2²+V3²+V4²+V5²) / V1 × 100%
 ******************************************************************************
 */

#include "fft_analyzer.h"
#include "oscilloscope.h"

/* CMSIS-DSP library */
#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"

/* Fallback PI definition if not provided by arm_math.h */
#ifndef PI
#define PI 3.14159265358979f
#endif

/* FFT input/output buffers (statically allocated) */
static float fft_input[FFT_POINT * 2];    /* Complex: real + imag interleaved */
static float fft_magnitude[FFT_BINS];     /* Magnitude spectrum */
static float fft_max_magnitude = 0;
static uint16_t fft_peak_bin = 0;
static float fft_peak_freq = 0;
static float fft_thd = 0;
static WindowType_t current_window = WINDOW_HANNING;

/* Hanning window coefficients (pre-computed for 1024 points) */
static float hanning_window[FFT_POINT];
static float hamming_window[FFT_POINT];
static float blackman_window[FFT_POINT];
static uint8_t windows_initialized = 0;

/**
 * @brief  Pre-compute window function coefficients
 */
static void init_windows(void)
{
    uint16_t i;
    for (i = 0; i < FFT_POINT; i++)
    {
        float n = (float)i;
        float N = (float)(FFT_POINT - 1);

        /* Hanning window: 0.5 * (1 - cos(2*pi*n / (N-1))) */
        hanning_window[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * n / N));

        /* Hamming window: 0.54 - 0.46 * cos(2*pi*n / (N-1)) */
        hamming_window[i] = 0.54f - 0.46f * arm_cos_f32(2.0f * PI * n / N);

        /* Blackman window: 0.42 - 0.5*cos(2*pi*n/(N-1)) + 0.08*cos(4*pi*n/(N-1)) */
        blackman_window[i] = 0.42f - 0.5f * arm_cos_f32(2.0f * PI * n / N)
                                      + 0.08f * arm_cos_f32(4.0f * PI * n / N);
    }
    windows_initialized = 1;
}

/**
 * @brief  Apply window function to input data
 */
static void apply_window(float *data, uint16_t count)
{
    uint16_t i;
    float *window = NULL;

    if (current_window == WINDOW_NONE)
        return;

    switch (current_window)
    {
    case WINDOW_HANNING:  window = hanning_window;  break;
    case WINDOW_HAMMING:  window = hamming_window;  break;
    case WINDOW_BLACKMAN: window = blackman_window; break;
    default: return;
    }

    for (i = 0; i < count && i < FFT_POINT; i++)
    {
        data[i] *= window[i];
    }
}

/**
 * @brief  Find the peak frequency bin in the magnitude spectrum
 *         Skips DC bin (index 0) and very low bins
 */
static void find_peak(uint32_t sample_rate)
{
    uint16_t i;
    float max_val = 0;
    uint16_t max_idx = 1;

    /* Skip first few bins (DC and very low frequencies) */
    for (i = 2; i < FFT_BINS; i++)
    {
        if (fft_magnitude[i] > max_val)
        {
            max_val = fft_magnitude[i];
            max_idx = i;
        }
    }

    fft_peak_bin = max_idx;
    fft_max_magnitude = max_val;

    /* Convert bin index to frequency: f = bin * sample_rate / FFT_POINT */
    fft_peak_freq = (float)max_idx * (float)sample_rate / (float)FFT_POINT;
}

/**
 * @brief  Calculate Total Harmonic Distortion (THD)
 *         THD = sqrt(V2² + V3² + V4² + V5²) / V1 × 100%
 *         Where V1 is the fundamental, V2-V5 are harmonics 2-5
 */
static void calculate_thd(void)
{
    uint16_t fundamental = fft_peak_bin;
    float v1, v2, v3, v4, v5;
    float harmonic_sum;

    if (fundamental < 2)
    {
        fft_thd = 0;
        return;
    }

    /* Get fundamental and harmonic magnitudes */
    v1 = fft_magnitude[fundamental];

    /* Harmonic 2 */
    uint16_t h2 = fundamental * 2;
    v2 = (h2 < FFT_BINS) ? fft_magnitude[h2] : 0;

    /* Harmonic 3 */
    uint16_t h3 = fundamental * 3;
    v3 = (h3 < FFT_BINS) ? fft_magnitude[h3] : 0;

    /* Harmonic 4 */
    uint16_t h4 = fundamental * 4;
    v4 = (h4 < FFT_BINS) ? fft_magnitude[h4] : 0;

    /* Harmonic 5 */
    uint16_t h5 = fundamental * 5;
    v5 = (h5 < FFT_BINS) ? fft_magnitude[h5] : 0;

    if (v1 > 0)
    {
        harmonic_sum = v2 * v2 + v3 * v3 + v4 * v4 + v5 * v5;
        fft_thd = arm_sqrt_f32(harmonic_sum) / v1 * 100.0f;
    }
    else
    {
        fft_thd = 0;
    }
}

int8_t FFT_Analyze(const uint16_t *adc_data, uint16_t count, uint32_t sample_rate)
{
    uint16_t i;

    if (count == 0 || adc_data == NULL)
        return -1;

    /* Initialize window coefficients on first use */
    if (!windows_initialized)
    {
        init_windows();
    }

    /* Limit count to FFT_POINT */
    if (count > FFT_POINT)
        count = FFT_POINT;

    /* Step 1: Convert ADC data to float and normalize to [-1, 1] range
     * Fill into complex interleaved format: [real, imag, real, imag, ...]
     */
    for (i = 0; i < count; i++)
    {
        fft_input[2 * i] = (float)adc_data[i] / (float)ADC_MAX_VALUE - 0.5f;  /* Real */
        fft_input[2 * i + 1] = 0.0f;                                          /* Imag */
    }

    /* Zero-pad remaining if count < FFT_POINT */
    for (i = count; i < FFT_POINT; i++)
    {
        fft_input[2 * i] = 0.0f;
        fft_input[2 * i + 1] = 0.0f;
    }

    /* Step 2: Apply window function (only to real part) */
    {
        float temp_real[FFT_POINT];
        for (i = 0; i < FFT_POINT; i++)
        {
            temp_real[i] = fft_input[2 * i];
        }
        apply_window(temp_real, FFT_POINT);
        for (i = 0; i < FFT_POINT; i++)
        {
            fft_input[2 * i] = temp_real[i];
        }
    }

    /* Step 3: Perform 1024-point complex FFT using CMSIS-DSP */
    /* arm_cfft_sR_f32_len1024 is the twiddle factor table for 1024-point FFT */
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_input, 0, 1);

    /* Step 4: Calculate magnitude spectrum
     * arm_cmplx_mag_f32 takes complex input [real, imag, ...] and outputs magnitudes
     */
    arm_cmplx_mag_f32(fft_input, fft_magnitude, FFT_BINS);

    /* Step 5: Find peak frequency */
    find_peak(sample_rate);

    /* Step 6: Calculate THD */
    calculate_thd();

    /* Update global oscilloscope config */
    OscilloscopeConfig_t *cfg = Oscilloscope_GetConfig();
    cfg->fft_peak_freq = fft_peak_freq;
    cfg->fft_peak_mag = fft_max_magnitude;
    cfg->thd = fft_thd;

    return 0;
}

float *FFT_GetMagnitude(void)
{
    return fft_magnitude;
}

float FFT_GetMaxMagnitude(void)
{
    return fft_max_magnitude;
}

float FFT_GetPeakFrequency(void)
{
    return fft_peak_freq;
}

uint16_t FFT_GetPeakBin(void)
{
    return fft_peak_bin;
}

float FFT_GetTHD(void)
{
    return fft_thd;
}

void FFT_SetWindow(WindowType_t type)
{
    current_window = type;
}

WindowType_t FFT_GetWindow(void)
{
    return current_window;
}
