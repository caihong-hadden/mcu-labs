/**
 * @file main_15f.c
 * @brief STC15F 系列平台 (蓝桥杯 CT107D 架构) - 定时器0中断软件 PWM 实现 LED 呼吸灯
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz / 12.000MHz (STC 1T 模式高速单片机)
 *          硬件控制:
 *          1. 蓝桥杯板载外设采用 74HC138 译码器与 74HC573 锁存器管理总线。
 *          2. 将跳线 J13 置于 IO 模式，P4.2 输出低电平 (P42 = 0) 打开 74HC02 或非门通道。
 *          3. 设置 74HC138 输入引脚: P27=1, P26=0, P25=0 选中 Y4 输出低电平，
 *             经或非门反相后 Y4C 为高电平，导通 LED 锁存器。
 *          4. P0.0 对应板载指示灯 L1 (低电平点亮)。
 *          5. 采用 STC15 专属 1T 高速定时器0模式 (AUXR |= 0x80)，实现 100us 高精度硬件定时中断。
 */

#include <STC15F2K60S2.H>

/* PWM 与呼吸渐变控制变量 */
volatile unsigned char pwm_count = 0;    // PWM 时间片计数器 (0 ~ 99)
volatile unsigned char pwm_duty = 0;     // 当前高电平/点亮占空比 (0 ~ 100)
volatile unsigned char breath_count = 0; // 亮度调节节拍计数器
volatile unsigned char fade_up = 1;      // 渐变方向标志: 1-变亮, 0-变暗

/**
 * @brief 定时器0初始化函数 (1T 模式, 100us 定时中断 @11.0592MHz)
 */
void Timer0_Init(void)
{
    AUXR |= 0x80;   // 定时器0时钟为 1T 模式 (不分频，速度为传统51的12倍)
    TMOD &= 0xF0;   // 设置定时器0为模式0 (16位自动重装载)

    /* 11.0592MHz 1T 模式下: 100us 初值 = 65536 - (11059200 * 0.0001) = 64430 (0xFB2E) */
    TL0 = 0x2E;
    TH0 = 0xFB;

    TF0 = 0;        // 清除定时器0溢出中断标志
    ET0 = 1;        // 使能定时器0中断
    EA = 1;         // 开启全局总中断
    TR0 = 1;        // 启动定时器0
}

/**
 * @brief 定时器0中断服务函数
 * @note  每隔 100us 执行一次，负责刷新 PWM 占空比并输出到 P0.0 (L1)
 */
void timer0_isr(void) interrupt 1
{
    pwm_count++;

    /* 周期达到 100 个时间片 (100 * 100us = 10ms, 即 100Hz 刷新频率) */
    if (pwm_count >= 100)
    {
        pwm_count = 0;

        /* 每隔 20ms (2个PWM周期) 调整一级占空比 */
        breath_count++;
        if (breath_count >= 2)
        {
            breath_count = 0;

            if (fade_up)
            {
                if (pwm_duty < 100)
                    pwm_duty++;  // 渐亮
                else
                    fade_up = 0; // 达到最大亮度后转为渐暗
            }
            else
            {
                if (pwm_duty > 0)
                    pwm_duty--;  // 渐暗
                else
                    fade_up = 1; // 达到最暗后转为渐亮
            }
        }
    }

    /* 
     * 74HC573 锁存器已在 main 中选通使能 (P42=0, P27=1, P26=0, P25=0)
     * 此时 P0 数据直接透明输出至 L1~L8
     * L1 对应 P0.0，低电平点亮，高电平熄灭；其余位全部保持高电平熄灭
     */
    if (pwm_count < pwm_duty)
    {
        P0 = 0xFE;  // 1111 1110B: L1 点亮，L2~L8 熄灭
    }
    else
    {
        P0 = 0xFF;  // 1111 1111B: 全部熄灭
    }
}

/**
 * @brief 主函数
 */
void main(void)
{
    /* 1. 打开 74HC138 译码门禁与 LED (Y4C) 锁存器通道 */
    P42 = 0;        // WR 门禁使能 (J13 IO 模式)
    P27 = 1;
    P26 = 0;
    P25 = 0;        // 选中 Y4C (LED 锁存器 74HC573 LE 处于高电平透明状态)

    /* 2. 初始状态关闭所有 LED */
    P0 = 0xFF;

    /* 3. 启动高精度 1T 定时器0中断 */
    Timer0_Init();

    while (1)
    {
        /* 主循环空闲，中断驱动平滑呼吸灯展示 */
    }
}
