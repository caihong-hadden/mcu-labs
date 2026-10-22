# STM32G474 电赛代码库（电源类重点）

> 收集整理于 2026-07-28，面向 **全国大学生电子设计竞赛**，主攻 **电源类赛题**，主控芯片 **STM32G474**（Cortex-M4 170MHz，模拟增强型，含 HRTIM 高分辨率定时器、运放、比较器）。

---

## 一、项目总览

本目录共收录 **13 个项目**（分 6 大类），从外设驱动、控制算法到各电源拓扑实现，系统覆盖电源类电赛需求。

| # | 目录 | 来源 | 大小 | 定位 |
|---|------|------|------|------|
| 01 | `01_G474外设例程库` | m-nabidoust | 71M | G474 GPIO 入门例程 |
| 02 | `02_G474数控电源` | Xuanzheng0901 ⭐ | 91M | **基础平台**：PID/滤波/ADC/OLED/FreeRTOS |
| 03 | `03_G474_FOC电机控制` | hongkou-chaozi ⭐ | 247M | **外设配置参考**：全套 .ioc + Keil 工程 |
| 04 | `04_OLED驱动库_SSD1306` | afiskon | 2.2M | 通用 OLED 驱动 |
| 05 | `05_电赛参考_2025A题变流器` | qqws1352 | 22M | SVPWM/双闭环架构参考(F407标准库) |
| 06 | `06_电源类专项/` | Xuanzheng0901 + 补充 ⭐⭐ | 661M | **8 个电源拓扑实现**（见下） |

### 06_电源类专项 子目录（电源类核心）

| 子目录 | 来源分支 | 大小 | 电源类型 | 独有算法模块 |
|--------|---------|------|---------|-------------|
| `APFC_有源功率因数校正` | APFC | 147M | AC-DC 并网 | **SOGI锁相环** + PFC + ctrl_loop ⭐ |
| `PFC_功率因数校正` | PFC | 91M | AC-DC | 功率因数校正 |
| `DCAC_逆变器` | dcac | 91M | DC-AC | 逆变控制 |
| `MPPT_最大功率跟踪` | MPPT | 91M | 光伏 DC-DC | **MPPT 扰动观察法** ⭐ |
| `DAB_双有源桥` | DAB | 92M | 隔离 DC-DC | 双有源桥移相控制 |
| `DAB-SPS_双有源桥单移相` | DAB-SPS | 92M | 隔离 DC-DC | 单移相控制 |
| `G474_HRTIM_三相SPWM逆变器` | nihinihikun | 253K | DC-AC | **HRTIM 三相 SPWM**（精简参考）⭐ |
| `STM32_SPWM波形生成` | wangqunzhu | 60M | DC-AC | 三相互补 SPWM |

> 总计约 1.1G。每个项目保留 `.git`，可查版本历史，不需要时可删除以省空间。

---

## 二、电源类拓扑覆盖矩阵

电源类赛题常见拓扑与算法的覆盖情况：

| 电源类型 / 算法 | 02 基础 | 06-APFC | 06-PFC | 06-DCAC | 06-MPPT | 06-DAB | 06-HRTIM逆变器 | 05 电赛参考 |
|----------------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Buck/Boost/Buck-Boost** | ✅ | — | — | — | — | — | — | — |
| **DC-AC 逆变器** | — | — | — | ✅ | — | — | ✅ | ✅ |
| **PFC 功率因数校正** | — | ✅ | ✅ | — | — | — | — | — |
| **APFC + 并网** | — | ✅ | — | — | — | — | — | — |
| **MPPT 光伏跟踪** | — | — | — | — | ✅ | — | — | — |
| **隔离 DC-DC (DAB)** | — | — | — | — | — | ✅ | — | — |
| **SOGI 锁相环 PLL** | — | ✅ | — | — | — | — | — | — |
| **SVPWM 空间矢量** | — | — | — | — | — | — | — | ✅ |
| **SPWM 正弦脉宽** | — | — | — | ✅ | — | — | ✅ | — |
| **双闭环 (电压+电流)** | ✅ | ✅ | — | ✅ | — | — | — | ✅ |
| **PID (增量/位置式)** | ✅ | ✅ | ✅ | ✅ | — | ✅ | — | ✅ |
| **卡尔曼滤波** | ✅ | ✅ | ✅ | ✅ | — | ✅ | — | — |
| **HRTIM 高分辨率 PWM** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | — |
| **ADC DMA 双缓冲** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | — | ✅ |
| **OLED 显示** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | — | ✅ |
| **FreeRTOS** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | — | — |

---

## 三、核心算法模块定位（电源类重点）

电源类赛题需要的核心算法，直接定位到代码文件：

### 锁相环 PLL（并网/同步必备）
- **SOGI 二阶广义积分器锁相环** → `06_电源类专项/APFC_有源功率因数校正/User/components/PLL/sogi.c` & `sogi.h`
- 原理：用 SOGI 提取电网正序分量，输出相位角 θ，实现单位功率因数并网

### 双闭环控制（电压外环 + 电流内环）
- **CV/CC 双环** → `02_G474数控电源/User/components/pid_ctrl/`（增量式+位置式+抗积分饱和）
- **APFC 控制环路** → `06_电源类专项/APFC/User/components/ctrl_loop/`（ctrl_loop.c + ctrl_loop_state.c，含状态机）

### PFC 功率因数校正
- → `06_电源类专项/APFC_有源功率因数校正/User/components/PFC/`（pfc_config.h + pfc_utils.c）

### MPPT 最大功率点跟踪
- **扰动观察法** → `06_电源类专项/MPPT_最大功率跟踪/User/components/MPPT/`（mppt.h + mppt_routine.c）

### SPWM / SVPWM 波形生成
- **HRTIM 三相 SPWM（G474 专用）** → `06_电源类专项/G474_HRTIM_三相SPWM逆变器/src/Core/Src/main.c`（LUT 查表 + HRTIM 死区）
- **三相互补 SPWM（TIM8）** → `06_电源类专项/STM32_SPWM波形生成/`
- **SVPWM 查表法（架构参考）** → `05_电赛参考_2025A题变流器/HARDWARE/`（⚠️ F407 标准库，仅参考算法）

### HRTIM 高分辨率定时器配置
- **G474.ioc 完整配置** → `03_G474_FOC电机控制/G474.ioc` + `Core/Src/hrtim.c`（CubeMX 参数可借鉴）
- **HRTIM 精简例程** → `06_电源类专项/G474_HRTIM_三相SPWM逆变器/`（Nucleo-G474RE，253K 轻量）

### 滤波算法
- **一维卡尔曼滤波** → `02_G474数控电源/User/components/filter/kalman.c`（可调 Q/R）
- **CMSIS-DSP (FIR/IIR/FFT)** → 参考 `05_电赛参考/HARDWARE/ArmMath/`

### ADC 多通道 DMA 采样
- → `02_G474数控电源/User/components/ADC/`（ADC1~4，DMA 双缓冲，差分/单端）

### 保护逻辑（过流/过压/孤岛）
- 参考 05 项目的保护检测框架（关 PWM + 报警）

---

## 四、各项目详细说明

### 02_G474数控电源 ⭐ 基础平台
- **来源**：github.com/Xuanzheng0901/STM32G474-DigitalPower (master 分支)
- **芯片**：STM32G474RET6
- **内容**：Buck-Boost 数字电源基础平台，CV/CC 双环 PID 控制
- **`User/components/` 核心模块**：ADC(DMA双缓冲) / pid_ctrl(增量+位置式) / filter(卡尔曼) / display(OLED+LVGL) / LOG / console / UI
- **价值**：所有 06 电源分支的**公共基础框架**，PID/滤波/ADC/OLED 模块可直接拷贝复用

### 06_电源类专项 ⭐⭐ 电源类核心
均来自同一仓库 (Xuanzheng0901/STM32G474-DigitalPower) 的不同分支，共享 02 的基础平台，各分支差异在 `User/components/` 的独有模块：

#### APFC_有源功率因数校正（最完整，并网首选）
- **独有模块**：`PLL/`(SOGI锁相环) + `PFC/` + `ctrl_loop/`(控制环路+状态机)
- **适用**：并网逆变器、APFC 整流、需要电网同步的题目
- **亮点**：SOGI-PLL 是并网单位功率因数控制的核心算法

#### MPPT_最大功率跟踪
- **独有模块**：`MPPT/`(mppt_routine.c，扰动观察法)
- **适用**：光伏发电、太阳能充电类题目

#### DCAC_逆变器
- **基础模块**：基于 02 平台的 DC-AC 逆变实现
- **适用**：单相/三相逆变、正弦波输出

#### PFC / APFC
- PFC：基础功率因数校正；APFC：有源 PFC + 锁相环（更完整）

#### DAB / DAB-SPS（双有源桥）
- 隔离型 DC-DC 变换器，适合双向能量传输题目
- DAB：完整移相控制；DAB-SPS：单移相（简化版）

#### G474_HRTIM_三相SPWM逆变器（独立项目，轻量）
- **来源**：github.com/nihinihikun/STM32_NIHI_SimpleInverter
- **硬件**：Nucleo-G474RE
- **内容**：开环三相 SPWM，HRTIM 生成 PWM + 硬件死区，LUT 查表正弦合成
- **参数**：基波 50Hz，开关 20kHz，死区 50ns，调制比 0.85
- **价值**：253K 精简工程，快速理解 G474 HRTIM SPWM 实现，CubeIDE 直接打开

#### STM32_SPWM波形生成
- **来源**：github.com/wangqunzhu/STM32-SPWM
- **内容**：用 TIM8 输出三相互补 SPWM（各差120°），含死区配置
- **价值**：非 HRTIM 的普通高级定时器 SPWM 方案，适用 F1/F4/G4

### 其他项目（01/03/04/05）
- **01_G474外设例程库**：⚠️ README 描述完整但实际仅 GPIO 例程，外设配置看 03
- **03_G474_FOC电机控制** ⭐：`G474.ioc` + Core/Src 全套外设初始化(adc/hrtim/tim/dac/dma/spi/i2c/usart/cordic)，Keil 工程，外设配置最佳参考
- **04_OLED驱动库**：afiskon 通用 SSD1306/SH1106 驱动，I2C+SPI，移植简单
- **05_电赛参考_2025A题变流器**：⚠️ STM32F407 标准库，仅作 SVPWM/双闭环架构参考，不能直接用于 G474

---

## 五、按赛题类型推荐组合

| 赛题类型 | 主用项目 | 关键模块 |
|---------|---------|---------|
| **DC-DC 稳压电源** | 02 + 03 | 02 的 PID双环 + ADC DMA；03 的 HRTIM/TIM 配置 |
| **单相逆变电源** | 06-DCAC + 06-HRTIM逆变器 | DCAC 控制框架 + HRTIM SPWM 参考 |
| **三相逆变** | 06-HRTIM逆变器 + 05 | G474 HRTIM 三相 SPWM + 05 的 SVPWM 架构 |
| **并网逆变器** | 06-APFC ⭐ | SOGI锁相环 + PFC + ctrl_loop 控制环路 |
| **功率因数校正** | 06-APFC 或 06-PFC | APFC 更完整（含 PLL） |
| **光伏 MPPT** | 06-MPPT + 02 | MPPT 扰动观察法 + 02 的 PID/ADC 平台 |
| **隔离 DC-DC** | 06-DAB | 双有源桥移相控制 |
| **能量回馈** | 05 + 06-APFC | 05 的 SVPWM 架构 + APFC 的并网同步 |

---

## 六、开发环境

| 工具 | 用途 |
|------|------|
| **STM32CubeMX** | 图形化配置外设，打开 .ioc 文件 |
| **Keil MDK-ARM** | 03 项目用，编译/调试 |
| **STM32CubeIDE** | 01/06-HRTIM逆变器 用，免费 |
| **CMake + Ninja + arm-gcc** | 02/06 各分支命令行构建 |
| **ST-Link / J-Link** | 烧录调试 |

---

## 七、补充学习资源

- **单相并网逆变器完整教程**（含 PLL/双环/SPWM/保护/MPPT 全套 C 代码）：
  cnblogs.com/t435hh18/p/19716973 — 算法原理清晰，代码可移植
- **ST 官方 STM32CubeG4**：github.com/STMicroelectronics/STM32CubeG4（最权威外设例程）
- **HRTIM 同步 TIM1 相移 PWM 触发 ADC**：搜 ST 文档 "LAT1451"
- **CMSIS-DSP 库**：FIR/IIR/FFT，配合 G474 的 FPU + CORDIC 做高效信号处理

---

## 八、备战建议

STM32G474 做电源类电赛的优势：
- **HRTIM**（184ps 分辨率）：高频电源核心，DC-DC/DC-AC/逆变器首选
- **4 个 ADC + DMA**：同步采样多路电压电流，高吞吐
- **内置运放 + 比较器**：省外部器件，信号调理 + 硬件过流保护
- **CORDIC + FMAC**：硬件加速三角运算和数字滤波

推荐工作流：
1. **CubeMX 配置外设**（参考 03 的 G474.ioc）
2. **拷贝 02 的基础模块**（PID/滤波/ADC/OLED）做底座
3. **按题目类型选 06 对应分支**，参考其独有算法模块
4. **并网类必看 APFC 的 SOGI-PLL**

_祝电赛顺利！_
