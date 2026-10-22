# STM32G474 CubeMX 配置指南（AC-AC 变换电路）

> 面向 2026 电赛 A 题，主控 STM32G474CET6（LQFP48）。手把手配置，照着做即可。
> 配置参考 APFC 分支 `G474_1.ioc`（已验证的电源类配置）。

---

## 第 0 步：准备工作

1. 下载安装 **STM32CubeMX**（st.com 免费，版本 6.6+）
2. 准备好你的板子原理图（确认引脚映射）
3. 打开 CubeMX，点 **ACCESS TO MCU SELECTOR**

---

## 第 1 步：选芯片

1. 搜索框输入 `STM32G474CE`
2. 选 **STM32G474CETx**（LQFP48）
3. 点右上角 **Start Project**

---

## 第 2 步：配置时钟（目标 168MHz）

1. 左侧 **System Core → RCC**
   - **High Speed Clock (HSE)**: 选 `Crystal/Ceramic Resonator`（你板子有 8MHz 晶振）
   - **Low Speed Clock (LSE)**: 如果用了 RTC（CR1220），选 `Crystal/Ceramic Resonator`
2. 顶部 **Clock Configuration** 标签页：
   - Input frequency: `8 MHz`（你板子的晶振）
   - PLL Source Mux: 选 **HSE**
   - **PLLN = 42**, PLLM = 1, PLLR = 2
   - System Clock Mux: 选 **PLLCLK**
   - **HCLK = 168** MHz（回车，CubeMX 自动算分频）
   - 确认 HRTIM1 Clock = 168MHz, ADC Clock = 168MHz

> 为什么 168 不是 170？APFC 原工程用 168MHz，HRTIM 4 倍频 = 672MHz，分频整齐。你跟着用就行。

---

## 第 3 步：配置 HRTIM（核心！PWM 输出）

HRTIM 是 G474 的杀手锏，这道题要两组：
- **PFC 级**：1 路 PWM（Boost 开关管）→ Timer A
- **逆变级**：3 组互补 PWM（三相桥）→ Timer B/C/D

### 3.1 启用 HRTIM
1. 左侧 **Timers → HRTIM1**，勾选 **Master Timer** + **Timer A/B/C/D**
2. 每个Timer勾选输出通道

### 3.2 关键参数（每个 Timer 都设）
| 参数 | 值 | 说明 |
|------|-----|------|
| Prescaler Ratio | `MUL4` | 4倍频，168M×4=672MHz 计数频率 |
| Period | `PWM_Period` | 用宏名，在代码里 `#define PWM_Period 28000`（约 24kHz 开关频率） |
| Preload Enable | ✓ | 影子寄存器，改占空比本周期内生效 |

### 3.3 逆变级互补 PWM（Timer B/C/D）
- 每个 Timer 设 **互补输出**（如 TB1+TB2）
- **死区时间**：Dead Time 设 `500`（约 50ns，根据你 MOS 管调整，防直通炸管）

### 3.4 PFC 级 PWM（Timer A）
- 单路输出 TA1
- HRTIM 触发 ADC：在 ADC 配置里关联

> **开关频率建议 20-50kHz**。太高损耗大影响效率，太低滤波难。APFC 工程用约 24kHz。
> HRTIM 计数频率 672MHz，Period=28000 → 672M/28000 = 24kHz。

---

## 第 4 步：配置 ADC（多通道 + HRTIM 同步触发）

### 4.1 启用 ADC
- 左侧 **Analog → ADC1**，勾选需要的通道（IN1~IN5 等）
- 这道题至少 5 路采样：
  - 输入电压 Ui、输入电流 Ii（PFC 用）
  - 直流母线电压 Vdc（PFC 电压外环）
  - 输出线电压 Uab、Ubc（逆变闭环）

### 4.2 关键参数
| 参数 | 值 | 说明 |
|------|-----|------|
| Continuous Conversion | **Disable** | 用 HRTIM 触发，不用连续 |
| External Trigger | **HRTIM_TRG1** | HRTIM 触发，与 PWM 同步采样 |
| Trigger Edge | **Rising** | 上升沿触发 |
| Sampling Time | `12.5 Cycles` | 168MHz/12.5 ≈ 短采样，够用 |
| DMA Request | **勾选** | 配 DMA 双缓冲（见第 5 步） |

### 4.3 通道顺序
按 Rank 顺序排好：Rank1=Ui, Rank2=Ii, Rank3=Vdc, Rank4=Uab, Rank5=Ubc
> HRTIM 每触发一次，ADC 自动按 Rank 顺序转完 5 路，DMA 搬到内存。

---

## 第 5 步：配置 DMA（ADC 数据搬运）

1. ADC1 配置页 → **DMA Settings** → Add
2. 参数：
   - **Mode: Circular**（循环）
   - **Data Width**: Peripheral Half Word(16bit), Memory Half Word
   - **DMA Buffer**: 设 5 的倍数（如 10，双缓冲 5 通道×2 组）

---

## 第 6 步：配置 USART（串口调试）

1. 左侧 **Connectivity → USART3**（或你板子引出的串口）
2. Mode: **Asynchronous**
3. Baud Rate: **115200**
4. 引脚看原理图（USART3_TX/RX 或 USART2）

> 串口用来 printf 调试 + 实时传参（APFC 的 console 模块就是这个）

---

## 第 7 步：配置 I2C（OLED 显示）

1. 左侧 **Connectivity → I2C1**
2. Mode: **I2C**
3. 引脚：SCL/SDA 看原理图（你板子 OLED 是 I2C 接口）

> 02 数控电源项目的 OLED 驱动（SSD1306/SH1107）直接复用

---

## 第 8 步：配置 FreeRTOS（可选但推荐）

1. 左侧 **Middleware → FREERTOS**
2. Interface: **CMSIS_V2**
3. 勾选 **USE_FPU**（G474 有硬件浮点，必须开）
4. **Tasks** 标签：新建任务
   - Task1: `PFC_Ctrl`（PFC 控制环，高优先级）
   - Task2: `Inv_Ctrl`（逆变控制环，高优先级）
   - Task3: `Display`（OLED 刷新，低优先级）
   - Task4: `Console`（串口调参，低优先级）
5. Heap Size: **16384**（16KB，够用）

> 注意：HRTIM 中断回调里做控制运算（实时性要求高），FreeRTOS 任务做显示/通信。
> 控制环建议放 **HRTIM 中断**里，不要放任务里！

---

## 第 9 步：配置 SYS + 中断

1. **System Core → SYS** → Debug: **Serial Wire**（SWD 调试）
2. **NVIC Settings**（中断）标签：
   - **HRTIM1 Master IRQ**: 最高优先级（控制环在这里跑）
   - **DMA1 Channel1 IRQ**: 高优先级（ADC DMA 完成）
   - **USART3 IRQ**: 中优先级
   - FreeRTOS 的 PendSV/SVC 保持默认（最低优先级）

---

## 第 10 步：生成工程

1. **Project Manager** 标签：
   - Project Name: `AC_AC_Converter`
   - Toolchain: **MDK-ARM V5**（用 Keil）或 **STM32CubeIDE**
   - Code Generator: 勾 **Generate peripheral initialization as pair of .c/.h files**
   - Advanced Settings: 勾 **将所有外设初始化分散到独立文件**
2. 右上角 **GENERATE CODE**

---

## 第 11 步：集成算法模块（代码合并）

生成代码后，把之前下载的模块拷进来：

| 来源 | 拷贝到 | 作用 |
|------|--------|------|
| `02/User/components/pid_ctrl/` | 你的工程/Core/Src | PID 算法 |
| `02/User/components/filter/kalman.c/h` | 你的工程/Core/Src | 卡尔曼滤波 |
| `06/APFC/User/components/PLL/sogi.c/h` | 你的工程/Core/Src | **SOGI 锁相环**（已仿真验证） |
| `06/APFC/User/components/ctrl_loop/` | 你的工程/Core/Src | 双闭环控制框架 |
| `02/User/components/display/` | 你的工程/Core/Src | OLED 驱动 |
| `02/User/components/console/` | 你的工程/Core/Src | 串口调参 |

### 关键代码插入点
- `main.c` 的 `while(1)` 前：初始化 SOGI_PLL（`SOGI_init()`）、PID、OLED
- `stm32g4xx_it.c` 的 `HRTIM1_IRQHandler` → 调用 `HAL_HRTIM_IRQHandler` → 在回调里：
  1. 读 ADC DMA 缓冲（5 路采样值）
  2. 跑 SOGI-PLL（输入 Ui）
  3. 跑 PFC 双闭环（输出 PFC PWM 占空比）
  4. 跑逆变电压闭环（输出三相 SVPWM 占空比）
  5. 更新 HRTIM 比较寄存器

---

## 引脚映射速查（CET6，按你的板子原理图调整）

> ⚠️ 你的板子 ADC 只引出 2 路，这题要 5 路。**必须飞线引出更多 ADC 引脚，或外挂 SPI ADC 芯片**。

| 功能 | 推荐引脚 | 外设 | 备注 |
|------|---------|------|------|
| PFC PWM | PA8 | HRTIM_TA1 | Boost 开关管 |
| 逆变 A 相上/下管 | PA9/PA10 | HRTIM_TB1/TB2 | 互补+死区 |
| 逆变 B 相上/下管 | PA11/PA12 | HRTIM_TC1/TC2 | 互补+死区 |
| 逆变 C 相上/下管 | PB14/PB15 | HRTIM_TD1/TD2 | 互补+死区 |
| ADC Ui 输入电压 | PA0 | ADC1_IN1 | PFC 用 |
| ADC Ii 输入电流 | PA1 | ADC1_IN2 | PFC 用 |
| ADC Vdc 母线电压 | PA2 | ADC1_IN3 | PFC 外环 |
| ADC Uab 输出电压 | PA3 | ADC1_IN4 | 逆变闭环 |
| ADC Ubc 输出电压 | PA4 | ADC1_IN5 | 逆变闭环 |
| USART3 TX/RX | PB10/PB11 | USART3 | 调试串口 |
| I2C1 SCL/SDA | PB6/PB7 | I2C1 | OLED |
| SWDIO/SWCLK | PA13/PA14 | SYS | 调试下载 |

> 具体引脚必须对照你板子的原理图！HRTIM 引脚是固定的（见 G474 数据手册 alternate function 表）。

---

## 常见坑

1. **HRTIM 不输出**：检查 Repetition Update 是否使能（不使能需软件触发一次 update）
2. **ADC 不触发**：确认 HRTIM 的 ADC Trigger Event 已使能（Timer 的事件映射到 ADC_TRG1）
3. **死区太小炸管**：先用 500ns 死区，示波器确认波形再减小
4. **FPU 没开**：FreeRTOS 里必须勾 USE_FPU，否则浮点运算异常慢
5. **ADC 通道不够**：CET6 引脚少，必要时用 ADC2/ADC3 并行，或外挂 SPI ADC

---

_配置完成后，烧录到板子，先用示波器看 HRTIM PWM 波形对不对，再接功率电路。_
