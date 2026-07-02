/**
 ******************************************************************************
 * @file    stm32f4xx_hal_msp.c
 * @brief   HAL MSP Initialization - Low-level hardware init for peripherals
 ******************************************************************************
 */

#include "main.h"

/* External handles */
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;
extern SRAM_HandleTypeDef hsram;

/**
 * @brief Global MSP initialization
 */
void HAL_MspInit(void)
{
    /* Set NVIC priority group */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* Memory management, no specific config needed */
}

/**
 * @brief ADC MSP Init - Configure ADC GPIO and DMA
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hadc->Instance == ADC1)
    {
        /* Enable clocks */
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        /* ADC1 Channel 0 (PA0) and Channel 1 (PA1) - Analog input */
        GPIO_InitStruct.Pin = ADC_CH1_PIN | ADC_CH2_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ADC_CH1_PORT, &GPIO_InitStruct);

        /* DMA2 Stream0 configuration for ADC1 */
        hdma_adc1.Instance = DMA2_Stream0;
        hdma_adc1.Init.Channel = DMA_CHANNEL_0;          /* ADC1 is on DMA2 Channel 0 */
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;          /* Increment memory pointer */
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* 16-bit */
        hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     /* 16-bit */
        hdma_adc1.Init.Mode = DMA_CIRCULAR;               /* Circular mode for continuous sampling */
        hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
        {
            Error_Handler();
        }

        /* Link DMA to ADC */
        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);

        /* Enable DMA interrupt */
        HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    }
}

/**
 * @brief TIM2 MSP Init - Input Capture GPIO for frequency measurement
 */
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* TIM2_CH1 - PA5 */
        GPIO_InitStruct.Pin = FREQ_IN_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(FREQ_IN_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

/**
 * @brief TIM Base MSP Init - For TIM3, TIM4, TIM6
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
    else if (htim->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
}

/**
 * @brief TIM Encoder MSP Init - TIM4 GPIO for encoder
 */
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM4)
    {
        __HAL_RCC_TIM4_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* TIM4_CH1 (PB6) and TIM4_CH2 (PB7) */
        GPIO_InitStruct.Pin = ENC_A_PIN | ENC_B_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
        HAL_GPIO_Init(ENC_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }
}

/**
 * @brief UART MSP Init
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9 (TX) and PA10 (RX) */
        GPIO_InitStruct.Pin = UART_TX_PIN | UART_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(UART_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/**
 * @brief SRAM (FSMC) MSP Init - LCD interface
 */
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable FSMC and GPIO clocks */
    __HAL_RCC_FSMC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* FSMC Data lines: D0-D15
     * D0=PD14, D1=PD15, D2=PD0, D3=PD1, D4=PE7, D5=PE8, D6=PE9, D7=PE10
     * D8=PE11, D9=PE12, D10=PE13, D11=PE14, D12=PE15, D13=PD8, D14=PD9, D15=PD10
     */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

    /* Port D pins: PD0(D2), PD1(D3), PD4(NOE), PD5(NWE), PD7(NE1), PD8(D13),
     *              PD9(D14), PD10(D15), PD11(A16), PD14(D0), PD15(D1) */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
                          GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                          GPIO_PIN_11 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Port E pins: PE7(D4), PE8(D5), PE9(D6), PE10(D7), PE11(D8),
     *              PE12(D9), PE13(D10), PE14(D11), PE15(D12) */
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                          GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                          GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}
