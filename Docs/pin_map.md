# 硬件引脚分配

## STM32F407VGT6 引脚映射

### ADC 信号采集
| 引脚 | 功能 | 外设 | 说明 |
|------|------|------|------|
| PA0 | ADC1_IN0 | ADC1 | 通道1 信号输入 |
| PA1 | ADC1_IN1 | ADC1 | 通道2 信号输入 |
| PA5 | TIM2_CH1 | TIM2 | 输入捕获（频率测量） |

### FSMC LCD 接口 (ILI9341, 16-bit)
| 引脚 | 功能 | FSMC信号 | 说明 |
|------|------|----------|------|
| PD0 | FSMC_D2 | D2 | 数据线 |
| PD1 | FSMC_D3 | D3 | 数据线 |
| PD4 | FSMC_NOE | NOE | 读使能 |
| PD5 | FSMC_NWE | NWE | 写使能 |
| PD7 | FSMC_NE1 | NE1 | 片选 (Bank1) |
| PD8 | FSMC_D13 | D13 | 数据线 |
| PD9 | FSMC_D14 | D14 | 数据线 |
| PD10 | FSMC_D15 | D15 | 数据线 |
| PD11 | FSMC_A16 | A16 | 命令/数据选择 (RS) |
| PD14 | FSMC_D0 | D0 | 数据线 |
| PD15 | FSMC_D1 | D1 | 数据线 |
| PE7 | FSMC_D4 | D4 | 数据线 |
| PE8 | FSMC_D5 | D5 | 数据线 |
| PE9 | FSMC_D6 | D6 | 数据线 |
| PE10 | FSMC_D7 | D7 | 数据线 |
| PE11 | FSMC_D8 | D8 | 数据线 |
| PE12 | FSMC_D9 | D9 | 数据线 |
| PE13 | FSMC_D10 | D10 | 数据线 |
| PE14 | FSMC_D11 | D11 | 数据线 |
| PE15 | FSMC_D12 | D12 | 数据线 |
| PB12 | GPIO Output | - | LCD 背光控制 |
| PB13 | GPIO Output | - | LCD 复位 |

### 编码器接口
| 引脚 | 功能 | 外设 | 说明 |
|------|------|------|------|
| PB6 | TIM4_CH1 | TIM4 | 编码器 A 相 |
| PB7 | TIM4_CH2 | TIM4 | 编码器 B 相 |
| PB8 | EXTI8 | EXTI | 编码器按键 (下降沿) |

### 用户按键
| 引脚 | 功能 | 说明 |
|------|------|------|
| PC13 | EXTI13 | KEY1 - 切换显示模式 |
| PC14 | EXTI14 | KEY2 - 切换触发模式 |
| PC15 | EXTI15 | KEY3 - 重置触发 |

### 调试串口
| 引脚 | 功能 | 外设 | 说明 |
|------|------|------|------|
| PA9 | USART1_TX | USART1 | 调试输出 (115200 8N1) |
| PA10 | USART1_RX | USART1 | 调试输入 |

## 时钟配置
```
HSE 8MHz → PLL (PLLM=8, PLLN=336, PLLP=2, PLLQ=7)
  → SYSCLK = 168MHz
  → AHB (HCLK) = 168MHz
  → APB1 (PCLK1) = 42MHz  (TIM clock = 84MHz)
  → APB2 (PCLK2) = 84MHz  (TIM clock = 168MHz, ADC clock = 21MHz)
```

## 中断优先级分配
| 优先级 | 外设中断 | 说明 |
|--------|----------|------|
| 0 (最高) | DMA2_Stream0 | ADC DMA 传输 |
| 1 | TIM2 | 输入捕获（频率测量） |
| 3 | TIM4 | 编码器 |
| 4 | TIM6 | 消抖定时器 |
| 5 | EXTI9_5 | 编码器按键 |
| 5 | EXTI15_10 | 用户按键 |
| 6 | USART1 | 调试串口 |
| 15 (最低) | SysTick | 系统滴答 |
