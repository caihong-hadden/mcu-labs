# 51 与 STM32 单片机实验库

> 从流水灯到 BLE 上报，二十个递进实验的完整工程代码。

## 这是什么

从点亮一个 LED 到做出能上报温湿度、能驱动舵机的小系统，中间隔着的不是知识点，而是**二十次「改了还能跑」的练习**。这个仓库是那二十次练习的完整代码。

## 递进路线

| 阶段 | 内容 |
| :--- | :--- |
| 入门 | 流水灯、按键消抖、蜂鸣器 |
| 外设 | 矩阵按键、数码管动态扫描、8×8 点阵字模、外部中断、定时器 |
| 综合 | 多外设协同、状态机式菜单 |
| 进阶 | STM32：AHT20 温湿度 + BLE 上报、舵机 PWM 控制 |

## 说明

- 每个 `demoN` 都是独立 Keil 工程，打开 `.uvprojx` 即可编译
- 仓库只保留源码与工程文件，编译产物（obj / hex / lst）已排除
- 部分工程引用厂商标准库，遵循其原始许可

## 技术栈

- **8051**：STC89C52 / STC15 系列，Keil C51
- **STM32**：STM32CubeIDE + HAL，AHT20（I2C）、BLE 串口透传、PWM 舵机
- **组织**：`demo1`–`demo20` 递进式实验，每个工程可独立编译

## 目录结构

```
  STM32/
    AHT20_BLE/
    DEMO2/
      DEMO2.ioc
    DEMO2_MARIO_BACKUP/
      DEMO2.ioc
    Demo1/
    SERVO_TEST/
  TC264_Crazy_Circuit_Pro/
    Tc264_Navigation_Demo_26_3_8/
    code/
      Encoder.c
      Encoder.h
      Eye_of_Providence.c
      Eye_of_Providence.h
      Imu660.c
      Imu660.h
      Motor.c
      Motor.h
      Parameters.c
      Parameters.h
      eeprom.c
      eeprom.h
      ... 另 4 个文件
    user/
      cpu0_main.c
      cpu0_main.h
      cpu1_main.c
      isr.c
      isr.h
      isr_config.h
  demo1/
    main.c
    main_15f.c
  demo10/
    main.c
    动态显示/
      main.c
    静态显示/
      main.c
  demo11/
    main.c
  demo12/
    Delay.c
    Delay.h
```

## 快速开始

Keil C51 打开对应 `demoN/*.uvprojx`；STM32 工程用 STM32CubeIDE 导入 `STM32/` 下的目录。

---

<sub>个人工程归档 · 2026 · 第三方库遵循其原始许可</sub>
