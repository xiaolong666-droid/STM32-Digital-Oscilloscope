/**
 ******************************************************************************
 * @file    stm32f4xx_hal_conf.h
 * @brief   HAL configuration file for STM32F407 Digital Oscilloscope
 ******************************************************************************
 */

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"

/* Module Selection - Enable modules used in this project */
#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_SRAM_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED

/* External oscillator value (Hz) */
#define HSE_VALUE    8000000U
#define HSE_STARTUP_TIMEOUT 5000U

/* Internal oscillator value (Hz) */
#define HSI_VALUE    16000000U
#define HSI_STARTUP_TIMEOUT 5000U

/* External low-speed oscillator */
#define LSE_VALUE    32768U
#define LSE_STARTUP_TIMEOUT 5000U
#define LSI_VALUE    32000U

/* VDD value */
#define VDD_VALUE                    3300U
#define TICK_INT_PRIORITY            15U
#define USE_RTOS                     0U
#define PREFETCH_ENABLE              1U
#define INSTRUCTION_CACHE_ENABLE     1U
#define DATA_CACHE_ENABLE            1U

/* HAL MPU region */
#define USE_HAL_MPU                  0U

/* Compile-time assertion */
#define STM32F407xx

/* Include HAL modules */
#ifdef HAL_RCC_MODULE_ENABLED
 #include "stm32f4xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
 #include "stm32f4xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
 #include "stm32f4xx_hal_dma.h"
#endif

#ifdef HAL_ADC_MODULE_ENABLED
 #include "stm32f4xx_hal_adc.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32f4xx_hal_tim.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
 #include "stm32f4xx_hal_uart.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
 #include "stm32f4xx_hal_pwr.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
 #include "stm32f4xx_hal_cortex.h"
#endif

#ifdef HAL_SRAM_MODULE_ENABLED
 #include "stm32f4xx_hal_sram.h"
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
 #include "stm32f4xx_hal_exti.h"
#endif

/* ADC EX */
#define HAL_ADC_EX_MODULE_ENABLED
#ifdef HAL_ADC_EX_MODULE_ENABLED
 #include "stm32f4xx_hal_adc_ex.h"
#endif

/* TIM EX */
#define HAL_TIM_EX_MODULE_ENABLED
#ifdef HAL_TIM_EX_MODULE_ENABLED
 #include "stm32f4xx_hal_tim_ex.h"
#endif

#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
