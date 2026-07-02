/**
 ******************************************************************************
 * @file    main.c
 * @brief   STM32F407 Digital Oscilloscope - Main Program
 *
 *          基于STM32F407VGT6的便携式数字示波器主程序
 *          功能：双通道信号采集、波形显示、FFT频谱分析、频率测量
 ******************************************************************************
 */

/* Includes ----------------------------------------------------------------*/
#include "main.h"
#include "bsp_adc.h"
#include "bsp_lcd.h"
#include "bsp_encoder.h"
#include "bsp_button.h"
#include "bsp_timer.h"
#include "ili9341.h"
#include "lcd_graphic.h"
#include "oscilloscope.h"
#include "ui_state_machine.h"
#include "frequency_counter.h"

/* Private variables ------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart1;
SRAM_HandleTypeDef hsram;

/* System Clock Configuration
 * HSE 8MHz -> PLL -> SYSCLK 168MHz
 * AHB = 168MHz, APB1 = 42MHz, APB2 = 84MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Initialize HSE and PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;     /* HSE 8MHz / 8 = 1MHz VCO input */
    RCC_OscInitStruct.PLL.PLLN = 336;   /* 1MHz * 336 = 336MHz VCO output */
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  /* 336/2 = 168MHz SYSCLK */
    RCC_OscInitStruct.PLL.PLLQ = 7;     /* 336/7 = 48MHz for USB */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Initialize CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;    /* HCLK = 168MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;     /* APB1 = 42MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;     /* APB2 = 84MHz */

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }

    /* Enable FSMC clock */
    __HAL_RCC_FSMC_CLK_ENABLE();
}

/**
 * @brief  GPIO initialization for non-FSMC, non-ADC pins
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable all GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* LCD Backlight (PB12) and Reset (PB13) */
    GPIO_InitStruct.Pin = LCD_BL_PIN | LCD_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_BL_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);   /* Backlight ON */
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET); /* Release reset */

    /* User Buttons (PC13, PC14, PC15) - External Interrupt falling edge */
    GPIO_InitStruct.Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStruct);

    /* Encoder Button (PB8) - External Interrupt falling edge */
    GPIO_InitStruct.Pin = ENC_BTN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ENC_PORT, &GPIO_InitStruct);

    /* Set interrupt priorities and enable */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 1);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief  ADC1 initialization - Dual channel (IN0/IN1), 12-bit, 1Msps
 *         Uses DMA2 Stream0 for continuous circular transfer
 */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;  /* 84/4 = 21MHz */
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;          /* Multi-channel scan */
    hadc1.Init.ContinuousConvMode = DISABLE;   /* Trigger by timer */
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO; /* TIM3 trigger */
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = ADC_CHANNELS;  /* 2 channels */
    hadc1.Init.DMAContinuousRequests = ENABLE;  /* DMA circular mode */
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure Channel 0 (PA0) - Rank 1 */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;  /* 15+12=27 cycles @21MHz ≈ 1.28us */
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure Channel 1 (PA1) - Rank 2 */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = 2;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Enable DMA double buffer mode */
    HAL_ADCEx_MultiModeConfigChannel(&hadc1, ADC_MULTIMODE_SINGL); /* Single mode, scan handles multi-channel */

    /* Start ADC with DMA */
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)bsp_adc_get_buffer(), ADC_DMA_BUF_SIZE) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  TIM2 - Input Capture for frequency measurement (PA5, TIM2_CH1)
 */
static void MX_TIM2_Init(void)
{
    TIM_IC_InitTypeDef sConfigIC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;                     /* No prescaler */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;               /* Max period for 32-bit timer */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    /* Input Capture Configuration - Channel 1, rising edge */
    sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;

    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  TIM3 - ADC trigger timer (triggers ADC at desired sample rate)
 *         Configured to generate TRGO update event at 1MHz / sample_rate
 */
static void MX_TIM3_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;            /* 84MHz / 1 = 84MHz */
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 83;              /* 84MHz / 84 = 1MHz trigger (1Msps) */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  TIM4 - Encoder interface mode (PB6, PB7)
 */
static void MX_TIM4_Init(void)
{
    TIM_Encoder_InitTypeDef sEncoderConfig = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 0xFFFF;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC1Filter = 10;    /* Hardware filter for debounce */
    sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC2Filter = 10;

    if (HAL_TIM_Encoder_Init(&htim4, &sEncoderConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  TIM6 - Debounce timer for buttons and encoder switch
 *         10ms period for debounce
 */
static void MX_TIM6_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 8399;          /* 84MHz / 8400 = 10kHz */
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 99;               /* 10kHz / 100 = 100Hz = 10ms */
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  USART1 - Debug output (115200 bps, 8N1)
 */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  FSMC (SRAM Bank1) initialization for LCD
 *         16-bit data bus, NE1 chip select, A16 as RS
 */
static void MX_FSMC_Init(void)
{
    FSMC_NORSRAM_TimingTypeDef Timing = {0};

    /* FSMC clock is already enabled in SystemClock_Config */

    hsram.Instance = FSMC_NORSRAM_DEVICE;
    hsram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
    hsram.Init.NSBank = FSMC_NORSRAM_BANK1;        /* NE1 */
    hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = FSMC_MEMORY_WIDTH_16B;
    hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

    /* Read/Write timing - tuned for ILI9341 */
    Timing.AddressSetupTime = 0;
    Timing.AddressHoldTime = 0;
    Timing.DataSetupTime = 2;    /* HCLK 168MHz -> ~12ns per cycle, 2 cycles = 24ns */
    Timing.BusTurnAroundDuration = 0;
    Timing.CLKDivision = 0;
    Timing.DataLatency = 0;
    Timing.AccessMode = FSMC_ACCESS_MODE_A;

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ======================== Application Entry Point ======================= */

int main(void)
{
    /* HAL initialization */
    HAL_Init();

    /* Configure system clock to 168MHz */
    SystemClock_Config();

    /* Initialize hardware peripherals */
    MX_GPIO_Init();
    MX_FSMC_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM6_Init();
    MX_USART1_UART_Init();

    /* Initialize BSP layer */
    bsp_adc_init();
    bsp_lcd_init();
    bsp_encoder_init();
    bsp_button_init();
    bsp_timer_init();

    /* Initialize ADC after BSP ADC buffer is ready */
    MX_ADC1_Init();

    /* Start timers */
    HAL_TIM_Base_Start(&htim3);              /* ADC trigger timer */
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);  /* Encoder */
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);      /* Frequency capture */
    HAL_TIM_Base_Start_IT(&htim6);           /* Debounce timer */

    /* Initialize LCD display */
    ILI9341_Init();
    LCD_SetBackColor(COLOR_BLACK);
    LCD_Clear(COLOR_BLACK);

    /* Initialize oscilloscope application */
    Oscilloscope_Init();

    /* Initialize frequency counter */
    frequency_counter_init();

    /* Initialize UI state machine */
    UI_StateMachine_Init();

    /* Main loop */
    while (1)
    {
        /* Process oscilloscope sampling and waveform update */
        Oscilloscope_Process();

        /* Process UI state machine (menu navigation, parameter changes) */
        UI_StateMachine_Process();

        /* Display update at target FPS */
        Oscilloscope_Display();
    }
}

/**
 * @brief  Error Handler - called on fatal errors
 *         Blinks LED and enters infinite loop
 */
void Error_Handler(void)
{
    /* Disable interrupts */
    __disable_irq();

    while (1)
    {
        /* If there's an LED, blink it here */
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add assertion failed message here */
}
#endif
