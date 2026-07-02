/**
 ******************************************************************************
 * @file    main.h
 * @brief   Header for main.c file
 *          This file contains the common defines and parameters for the
 *          STM32F407 Digital Oscilloscope project.
 ******************************************************************************
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* ======================== Pin Definitions =============================== */

/* ADC Channels - PA0 (CH1), PA1 (CH2) */
#define ADC_CH1_PIN     GPIO_PIN_0
#define ADC_CH1_PORT    GPIOA
#define ADC_CH2_PIN     GPIO_PIN_1
#define ADC_CH2_PORT    GPIOA

/* FSMC LCD Pins - 16-bit data bus + control signals */
/* Data: PD14(FSMC_D0), PD15(D1), PD0(D2), PD1(D3), PE7(D4), PE8(D5),
 *       PE9(D6), PE10(D7), PE11(D8), PE12(D9), PE13(D10), PE14(D11),
 *       PE15(D12), PD8(D13), PD9(D14), PD10(D15)
 * Control: PD4(NOE/RD), PD5(NWE/WR), PD7(NE1/CS), PD11(A16/RS)
 */
#define LCD_RS_PIN      GPIO_PIN_11   /* A16 - Register Select */
#define LCD_CS_PIN      GPIO_PIN_7    /* NE1 - Chip Select */
#define LCD_WR_PIN      GPIO_PIN_5    /* NWE - Write */
#define LCD_RD_PIN      GPIO_PIN_4    /* NOE - Read */
#define LCD_BL_PIN      GPIO_PIN_12   /* PB12 - Backlight (GPIO) */
#define LCD_BL_PORT     GPIOB
#define LCD_RST_PIN     GPIO_PIN_13   /* PB13 - Reset (GPIO) */
#define LCD_RST_PORT    GPIOB

/* Encoder - TIM4 CH1(PB6), CH2(PB7), Button(PB8) */
#define ENC_A_PIN       GPIO_PIN_6
#define ENC_B_PIN       GPIO_PIN_7
#define ENC_BTN_PIN     GPIO_PIN_8
#define ENC_PORT        GPIOB

/* User Buttons - PC13(KEY1), PC14(KEY2), PC15(KEY3) */
#define KEY1_PIN        GPIO_PIN_13
#define KEY2_PIN        GPIO_PIN_14
#define KEY3_PIN        GPIO_PIN_15
#define KEY_PORT        GPIOC

/* Frequency Counter Input - TIM2_CH1 (PA5) */
#define FREQ_IN_PIN     GPIO_PIN_5
#define FREQ_IN_PORT    GPIOA

/* USART1 Debug - PA9(TX), PA10(RX) */
#define UART_TX_PIN     GPIO_PIN_9
#define UART_RX_PIN     GPIO_PIN_10
#define UART_PORT       GPIOA

/* ======================== System Parameters ============================= */

#define SYSCLK_FREQ     168000000U   /* System clock frequency (Hz) */
#define ADC_CLK_FREQ    84000000U    /* APB2 clock (Hz) */

/* ADC Parameters */
#define ADC_RESOLUTION  12           /* 12-bit ADC */
#define ADC_VREF        3.3f         /* Reference voltage */
#define ADC_MAX_VALUE   4095         /* 2^12 - 1 */
#define ADC_SAMPLE_NUM  1024         /* Samples per channel for FFT */
#define ADC_CHANNELS    2            /* Dual channel */
#define ADC_DMA_BUF_SIZE (ADC_SAMPLE_NUM * ADC_CHANNELS)  /* 2048 half-words */

/* LCD Parameters */
#define LCD_WIDTH       320
#define LCD_HEIGHT      240
#define LCD_WAVEFORM_X  0
#define LCD_WAVEFORM_Y  0
#define LCD_WAVEFORM_W  240
#define LCD_WAVEFORM_H  200
#define LCD_INFO_X      240
#define LCD_INFO_Y      0
#define LCD_INFO_W      80
#define LCD_INFO_H      240
#define LCD_MENU_X      0
#define LCD_MENU_Y      200
#define LCD_MENU_W      240
#define LCD_MENU_H      40

/* FFT Parameters */
#define FFT_LENGTH      1024         /* Must match ADC_SAMPLE_NUM */

/* Display Update Rate */
#define DISPLAY_FPS     30
#define DISPLAY_INTERVAL_MS (1000 / DISPLAY_FPS)

/* ======================== Global Handles ================================ */

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim2;    /* Frequency counter */
extern TIM_HandleTypeDef htim3;    /* Display refresh timer */
extern TIM_HandleTypeDef htim4;    /* Encoder */
extern TIM_HandleTypeDef htim6;    /* Debounce timer */
extern UART_HandleTypeDef huart1;
extern SRAM_HandleTypeDef hsram;   /* FSMC for LCD */

/* ======================== Function Prototypes =========================== */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
