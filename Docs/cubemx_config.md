# STM32CubeMX 配置指南

本文件描述如何在 STM32CubeMX 中配置本项目所需的全部外设参数，生成基础工程后，再将 BSP/App/LCD 源码整合进去。

## 1. 新建工程

1. 打开 STM32CubeMX，选择 `STM32F407VGT6` 芯片
2. 工程名称：`Oscilloscope`
3. 工具链选择：`STM32CubeIDE` 或 `MDK-ARM` 或 `Makefile`

## 2. 时钟配置 (RCC)

```
HSE: Crystal/Ceramic Resonator (8MHz)
PLL Source: HSE
PLLM = 8
PLLN = 336
PLLP = 2 (168MHz SYSCLK)
PLLQ = 7
Flash Latency: 5 Wait States

HCLK (AHB)  = 168MHz
APB1 (PCLK1) = 42MHz  (TIM = 84MHz)
APB2 (PCLK2) = 84MHz  (TIM = 168MHz, ADC = 21MHz)
```

## 3. SYS 配置

- Debug: Serial Wire (SWD)
- Timebase Source: SysTick

## 4. NVIC 配置

- Priority Group: 4 bits for pre-emption priority
- 中断优先级参考 `Docs/pin_map.md`

## 5. GPIO 配置

| 引脚 | 模式 | 上拉/下拉 | 速度 | 标签 |
|------|------|-----------|------|------|
| PB12 | Output Push-Pull | No pull | High | LCD_BL |
| PB13 | Output Push-Pull | No pull | High | LCD_RST |
| PC13 | EXTI Falling | Pull-up | - | KEY1 |
| PC14 | EXTI Falling | Pull-up | - | KEY2 |
| PC15 | EXTI Falling | Pull-up | - | KEY3 |
| PB8 | EXTI Falling | Pull-up | - | ENC_BTN |
| PA0 | Analog | No pull | - | ADC_CH1 |
| PA1 | Analog | No pull | - | ADC_CH2 |
| PA9 | AF7 USART1_TX | Pull-up | Very High | - |
| PA10 | AF7 USART1_RX | Pull-up | Very High | - |

## 6. FSMC 配置 (LCD)

```
Mode: SRAM / PSRAM (Bank 1)
Bank: NE1 (Chip Select PD7)
Memory Type: SRAM
Data Width: 16-bit
Extended Mode: Disable

Write Timing:
  Address Setup Time: 0
  Data Setup Time: 2
  Access Mode: A

GPIO Alternate Function: AF12 (FSMC)
```

FSMC 引脚分配：
- PD0, PD1, PD4, PD5, PD7, PD8, PD9, PD10, PD11, PD14, PD15
- PE7, PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15

## 7. ADC1 配置

```
Clock Prescaler: DIV4 (21MHz ADC clock)
Resolution: 12-bit
Scan Conversion Mode: Enable
Continuous Conversion Mode: Disable
External Trigger: TIM3 TRGO (Rising Edge)
DMA Continuous Requests: Enable
EOC Selection: Single Conversion
NbrOfConversion: 2

Channel 0 (PA0): Rank 1, Sampling Time 15 cycles
Channel 1 (PA1): Rank 2, Sampling Time 15 cycles
```

## 8. DMA 配置 (ADC1)

```
DMA Request: ADC1
Stream: DMA2 Stream0
Direction: Peripheral to Memory
Mode: Circular
Peripheral Increment: Disable
Memory Increment: Enable
Peripheral Data Width: Half-Word (16-bit)
Memory Data Width: Half-Word (16-bit)
Priority: Very High
FIFO Mode: Disable
```

## 9. TIM3 配置 (ADC 触发定时器)

```
Clock Source: Internal Clock
Prescaler: 0
Counter Period: 83 (for 1Msps, 84MHz / 84 = 1MHz)
Auto-Reload Preload: Enable
Master/Slave Mode: Disable
TRGO: Update Event
```

## 10. TIM4 配置 (编码器)

```
Encoder Mode: Encoder Mode TI12
Channel1: PA6 -> PB6 (TIM4_CH1)
Channel2: PB7 (TIM4_CH2)
IC1 Filter: 10
IC2 Filter: 10
```

## 11. TIM2 配置 (频率测量输入捕获)

```
Clock Source: Internal Clock
Channel1: Input Capture direct mode (PA5)
Prescaler: 0
Counter Period: 0xFFFFFFFF
IC Polarity: Rising Edge
IC Selection: Direct TI
IC Prescaler: Div1
IC Filter: 0
```

## 12. TIM6 配置 (消抖定时器)

```
Prescaler: 8399 (84MHz / 8400 = 10kHz)
Counter Period: 99 (10kHz / 100 = 100Hz = 10ms)
Auto-Reload Preload: Enable
```

## 13. USART1 配置 (调试串口)

```
Baud Rate: 115200
Word Length: 8 Bits
Stop Bits: 1
Parity: None
Mode: TX/RX
Flow Control: None
Over Sampling: 16 samples
```

## 14. 生成代码后整合

1. 将 `BSP/`, `App/`, `LCD/` 文件夹复制到生成的工程根目录
2. 在 IDE 中添加头文件包含路径：
   - `BSP/Inc`
   - `App/Inc`
   - `LCD/Inc`
3. 将 `BSP/Src/`, `App/Src/`, `LCD/Src/` 下的 `.c` 文件添加到编译列表
4. 在 `Drivers/` 目录下添加 CMSIS-DSP 库源码（从 STM32CubeF4 包获取）
5. 在编译器宏定义中添加 `ARM_MATH_CM4` 和 `__FPU_PRESENT=1`
6. 将 `main.c` 中的初始化代码整合到 CubeMX 生成的 main.c 中
7. 编译下载

## 15. CMSIS-DSP 库添加

CMSIS-DSP 库文件位置（在 STM32CubeF4 固件包中）：
```
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_f32.c
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix8_f32.c
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal.c
Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c
Drivers/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c
Drivers/CMSIS/DSP/Source/SupportFunctions/arm_fill_f32.c
Drivers/CMSIS/DSP/Source/SupportFunctions/arm_copy_f32.c
```

或者在 CubeIDE 中直接添加预编译的 libarm_cortexM4lf_math.a 静态库。
