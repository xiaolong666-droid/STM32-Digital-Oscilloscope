/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines
 ******************************************************************************
 */

#include "main.h"
#include "stm32f4xx_it.h"
#include "bsp_adc.h"
#include "bsp_encoder.h"
#include "bsp_button.h"
#include "bsp_timer.h"
#include "frequency_counter.h"

/* External handles from main.c */
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/

void NMI_Handler(void)
{
    while (1) { }
}

void HardFault_Handler(void)
{
    while (1) { }
}

void MemManage_Handler(void)
{
    while (1) { }
}

void BusFault_Handler(void)
{
    while (1) { }
}

void UsageFault_Handler(void)
{
    while (1) { }
}

void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/******************************************************************************/
/*                   External Interrupt Handlers                              */
/******************************************************************************/

/**
 * @brief EXTI9_5 - Encoder Button (PB8) + EXTI5~9
 */
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(ENC_BTN_PIN);
}

/**
 * @brief EXTI15_10 - User Buttons (PC13, PC14, PC15)
 */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY1_PIN);
    HAL_GPIO_EXTI_IRQHandler(KEY2_PIN);
    HAL_GPIO_EXTI_IRQHandler(KEY3_PIN);
}

/**
 * @brief DMA2 Stream0 - ADC1 DMA Transfer Complete / Half Transfer
 */
void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}

/**
 * @brief TIM2 - Input Capture for frequency measurement
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
 * @brief TIM3 - ADC trigger timer (no interrupt needed, hardware trigger)
 */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

/**
 * @brief TIM4 - Encoder (overflow interrupt for counter tracking)
 */
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}

/**
 * @brief TIM6 - Debounce timer (10ms periodic)
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}

/**
 * @brief USART1 - Debug UART
 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/******************************************************************************/
/*                      HAL Callback Functions                                */
/******************************************************************************/

/**
 * @brief GPIO EXTI callback - Button and Encoder switch events
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case ENC_BTN_PIN:
        bsp_encoder_btn_callback();
        break;
    case KEY1_PIN:
        bsp_button_callback(BUTTON_KEY1);
        break;
    case KEY2_PIN:
        bsp_button_callback(BUTTON_KEY2);
        break;
    case KEY3_PIN:
        bsp_button_callback(BUTTON_KEY3);
        break;
    default:
        break;
    }
}

/**
 * @brief ADC DMA Half Transfer Complete callback
 *        First half of buffer is ready for processing
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        bsp_adc_half_complete_callback();
    }
}

/**
 * @brief ADC DMA Transfer Complete callback
 *        Second half of buffer is ready for processing
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        bsp_adc_complete_callback();
    }
}

/**
 * @brief TIM2 Input Capture callback - Frequency measurement
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        frequency_counter_capture_callback();
    }
}

/**
 * @brief TIM6 Period elapsed callback - Debounce timer
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        bsp_timer_debounce_callback();
    }
}
