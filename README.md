# 基于 STM32F407 的便携式数字示波器

> 项目时间：2024.09 - 2025.01
> 硬件平台：STM32F407VGT6（主频 168MHz，1MB Flash + 192KB SRAM，内置 12bit ADC）

## 项目简介

基于 STM32F407VGT6 设计并实现的一款便携式数字示波器，支持双通道信号采集、波形实时显示、频率测量、FFT 频谱分析等功能。该项目完整实现了从 BSP 底层驱动到应用层人机交互的全栈开发。

## 功能特性

- **双通道同步采样**：使用 ADC1 的 2 个规则通道（IN0/IN1），配合 DMA2 Stream0 循环模式，实现最高 1Msps 双通道同步采样
- **实时波形显示**：基于 FSMC 接口驱动 ILI9341 TFT-LCD（320×240），帧率稳定 30fps，支持网格背景与局部刷新
- **FFT 频谱分析**：移植 CMSIS-DSP 库，1024 点复数 FFT，支持 Hanning 窗函数、频率峰值检测、THD 计算
- **频率测量**：基于输入捕获的频率计数功能
- **人机交互**：基于状态机的多级菜单，支持编码器旋钮（外部中断 + 定时器消抖）与按键双模式操作
- **多种触发模式**：自动、正常、单次触发
- **参数可调**：时基、电压档位、触发电平、通道开关等

## 硬件资源分配

| 外设 | 引脚 | 功能说明 |
|------|------|----------|
| ADC1_IN0 | PA0 | 通道1 信号输入 |
| ADC1_IN1 | PA1 | 通道2 信号输入 |
| FSMC_D0~D15 | PD14,PD15,PD0,PD1,PE7~PE15 | LCD 16位数据总线 |
| FSMC_NWE | PD5 | LCD 写信号 |
| FSMC_NOE | PD4 | LCD 读信号 |
| FSMC_NE1 | PD7 | LCD 片选 (Bank1) |
| FSMC_A16 | PD11 | LCD RS (命令/数据) |
| TIM4_CH1 | PB6 | 编码器 A 相 |
| TIM4_CH2 | PB7 | 编码器 B 相 |
| EXTI | PB8 | 编码器按键 |
| EXTI | PC13 | 用户按键 KEY1 |
| EXTI | PC14 | 用户按键 KEY2 |
| EXTI | PC15 | 用户按键 KEY3 |
| TIM2_CH1 | PA5 | 输入捕获（频率测量） |
| USART1 | PA9/PA10 | 调试串口 (115200bps) |

## 软件架构

```
STM32-Digital-Oscilloscope/
├── Core/                   # STM32 HAL 核心代码
│   ├── Inc/                # 核心头文件 (main.h, hal_conf, it.h)
│   └── Src/                # main.c, 中断服务, HAL MSP, system_init
├── BSP/                    # 板级支持包
│   ├── Inc/                # BSP 头文件
│   └── Src/                # ADC, LCD, Encoder, Button, Timer 驱动
├── App/                    # 应用层
│   ├── Inc/                # 应用头文件
│   └── Src/                # 示波器主控, 波形处理, FFT, UI菜单, 状态机
├── LCD/                    # LCD 显示驱动
│   ├── Inc/                # ILI9341, 字库, 图形库头文件
│   └── Src/                # ILI9341驱动, 字库, 图形绘制
├── Drivers/                # ST HAL 驱动 + CMSIS 库（已内置，无需下载）
│   ├── STM32F4xx_HAL_Driver/  # HAL 驱动源码和头文件
│   └── CMSIS/                # CMSIS Core + Device + DSP 库
├── MDK-ARM/                # Keil5 工程文件
│   ├── STM32Oscilloscope.uvprojx  # Keil5 工程文件（双击打开）
│   ├── STM32Oscilloscope.uvoptx   # Keil5 工程选项
│   ├── startup_stm32f407xx.s      # ARM 汇编启动文件
│   ├── STM32F407VGTx.sct          # 散列文件（内存布局）
│   └── retarget.c                  # 半主机重定向（禁用 semihosting）
├── Docs/                   # 项目文档（引脚映射、CubeMX配置）
└── LICENSE                 # MIT 开源协议
```

### 软件分层说明

1. **Core 层**：STM32 HAL 框架，包含系统时钟配置、中断向量、main 入口
2. **BSP 层**：板级硬件抽象，封装 ADC+DMA 采集、LCD FSMC 总线、编码器、按键、定时器
3. **LCD 层**：ILI9341 控制器驱动、ASCII 字库、基本图形绘制（画线、矩形、圆、文字）
4. **App 层**：示波器核心逻辑，包含波形数据处理、FFT 分析、频率测量、UI 状态机菜单

## 编译与烧录

### Keil MDK-ARM 5（已提供完整工程文件）

项目已包含完整的 Keil5 工程文件，位于 `MDK-ARM/` 目录下，Drivers/ 目录已内置 HAL 驱动和 CMSIS 库，可直接打开编译。

**步骤：**

1. 安装 Keil MDK 5.36+（从 https://www.keil.com/download/ 下载）
2. 安装 STM32F4xx DFP 器件支持包：
   - 打开 Keil → Pack Installer → 搜索 "STM32F4" → 安装 `Keil::STM32F4xx_DFP`
3. 打开 Keil5 → Project → Open Project → 选择 `MDK-ARM/STM32Oscilloscope.uvprojx`
4. 选择目标芯片 STM32F407VGT6（如未自动识别）
5. 编译（F7 / Build）
6. 连接 ST-Link V2 调试器 → 下载（F8 / Download）

**Keil5 工程配置说明：**
- 编译器：ARMCC V5.06（AC5），C99 模式，GNU 扩展
- 优化等级：-O1（可调整为 -O2 或 -O0 调试）
- 散列文件：`MDK-ARM/STM32F407VGTx.sct`（1MB Flash / 128KB RAM / 64KB CCM）
- 启动文件：`MDK-ARM/startup_stm32f407xx.s`（ARM 汇编格式）
- 预定义宏：`USE_HAL_DRIVER, STM32F407xx, ARM_MATH_CM4, __FPU_PRESENT=1U`
- 调试器：ST-Link III + SWO
- 已启用：生成 HEX 文件、下载后自动复位运行
- FPU：硬件浮点（Cortex-M4 SP-DP）

## 关键技术实现

### 1. ADC + DMA 双缓冲采集

采用 DMA 双缓冲机制，ADC1 以 1Msps 速率同时采样两个通道。DMA 配置为循环模式，使用半传输中断（HT）和传输完成中断（TC）实现乒乓缓冲：

- Buffer A 处理前半段数据时，ADC 写入 Buffer B
- 反之亦然，实现无缝采集

### 2. FFT 频谱分析

使用 ARM CMSIS-DSP 库的 `arm_cfft_f32` 函数进行 1024 点复数 FFT：

- 对采集数据施加 Hanning 窗减少频谱泄漏
- 计算幅度谱并检测峰值频率
- 计算 THD（总谐波失真）= sqrt(V2² + V3² + V4² + V5²) / V1 × 100%

### 3. TFT-LCD 局部刷新

基于 FSMC 8080 接口驱动 ILI9341，采用局部刷新策略：

- 仅重绘波形变化的区域，减少 SPI/FSMC 传输量
- 双波形缓存（前后帧对比），只擦除/绘制差异像素
- 网格背景预绘制到显存，不随波形刷新

### 4. 状态机 UI

采用层次化状态机管理多级菜单：

```
主界面 → [编码器按下] → 菜单界面
菜单界面 → 时基设置 / 电压设置 / 触发设置 / FFT设置 / 系统设置
```

## 开发环境

- **IDE**：STM32CubeIDE 1.13.0+ / Keil MDK 5.36+
- **CubeMX**：STM32CubeMX 6.9.0+
- **编译器**：arm-none-eabi-gcc 10.3+ / ARM Compiler 6
- **库版本**：STM32F4 HAL 1.27.1, CMSIS 5.6.0, CMSIS-DSP 1.10.0
- **烧录器**：ST-Link V2 / J-Link

## 个人职责

- 负责 STM32F407 的 BSP 开发：使用 STM32CubeMX 配置时钟树（SYSCLK 168MHz）、GPIO 复用、中断优先级分组，生成 HAL 工程框架
- 配置 ADC + DMA 双缓冲采集：使用 ADC1 的 2 个规则通道（IN0/IN1），配合 DMA2 Stream0 循环模式，实现最高 1Msps 双通道同步采样
- 编写 TFT-LCD 驱动层：基于 FSMC 接口驱动 ILI9341 控制器，实现像素级绘图、网格背景、波形缓存与局部刷新，帧率稳定 30fps
- 实现 FFT 频谱分析：移植 CMSIS-DSP 库，使用 1024 点复数 FFT，支持 Hanning 窗函数、频率峰值检测、THD 计算
- 设计人机交互界面：基于状态机实现多级菜单，支持编码器旋钮（外部中断 + 定时器消抖）与按键双模式操作

## License

MIT License
