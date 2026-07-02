/**
 ******************************************************************************
 * @file    stm32f4xx_it.h
 * @brief   Interrupt handlers header file
 ******************************************************************************
 */

#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* IRQ Handler prototypes */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* External interrupt handlers */
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

/* DMA handlers */
void DMA2_Stream0_IRQHandler(void);

/* Timer handlers */
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);

/* USART handler */
void USART1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */
