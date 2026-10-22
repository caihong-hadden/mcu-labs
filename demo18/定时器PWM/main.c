/**
 * @file main.c
 * @brief STC15F2K60S2 定时器1中断硬件 PWM 呼吸灯例程
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 12.000MHz (STC 1T 模式)
 *          核心原理:
 *          1. 定时器1配置为 100us 定时中断，100 个时间片构成 10ms (100Hz) PWM 周期;
 *          2. 在中断中自动调节占空比 duty (0~100)，输出至板载指示灯 L1 实现平滑呼吸。
 */

#include <STC15F2K60S2.H>

#define PWM_PERIOD 100 // PWM 周期总时间片数 (100 x 100us = 10ms)

unsigned char pwm_cnt = 0;   // PWM 计数器 (0~99)
unsigned char duty = 20;     // 当前占空比 (0~100)
bit dir = 0;                 // 呼吸渐变方向: 0-变亮, 1-变暗
unsigned int step_cnt = 0;   // 步进计时器

/**
 * @brief 定时器1初始化函数 (100us@12.000MHz, 1T 模式)
 */
void Timer1_Init(void)
{
    AUXR |= 0x40;       // 定时器1设为 1T 模式
    TMOD &= 0x0F;       // 16位自动重装载模式
    TL1 = 0x50;         // 65536 - 1200 = 64336 (0xFB50)
    TH1 = 0xFB;
    TF1 = 0;            // 清除溢出标志
    TR1 = 1;            // 启动定时器1
    ET1 = 1;            // 开启中断
    EA = 1;             // 开启总中断
}

void main(void)
{
    /* 开启 Y4C (LED 锁存通道) */
    P42 = 0;
    P27 = 1; P26 = 0; P25 = 0;
    P0 = 0xFF; // 初始全灭

    Timer1_Init(); // 启动 100us 中断

    while (1)
    {
        /* 主循环空闲，中断驱动 PWM 呼吸 */
    }
}

/**
 * @brief 定时器1中断服务函数 (100us 周期)
 */
void Timer1_Routine(void) interrupt 3
{
    pwm_cnt++;
    if (pwm_cnt >= PWM_PERIOD)
    {
        pwm_cnt = 0;
    }

    /* 比较占空比输出至 L1 (P0.0) */
    if (pwm_cnt < duty)
    {
        P0 = 0xFE; // 点亮 L1
    }
    else
    {
        P0 = 0xFF; // 熄灭 L1
    }

    /* 调节呼吸渐变速度 */
    step_cnt++;
    if (step_cnt >= 200) // 每 20ms 调整一级占空比
    {
        step_cnt = 0;
        if (dir == 0)
        {
            duty++;
            if (duty >= 100) dir = 1; // 达到最大亮度转向
        }
        else
        {
            duty--;
            if (duty <= 0) dir = 0;   // 达到最小亮度转向
        }
    }
}
