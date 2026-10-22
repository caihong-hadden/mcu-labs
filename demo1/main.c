/**
 * @file main.c
 * @brief 传统 8051 (REG52/STC89C52) 平台 - 定时器0中断软件 PWM 实现 LED 呼吸灯
 * @details 硬件平台: 传统 8051 / 89C52 开发板 (12T 模式, 晶振 12MHz)
 *          引脚连接: P1.0 引脚连接 LED 阴极 (低电平点亮)
 *          核心原理:
 *          1. 定时器0配置为模式2 (8位自动重装载), 产生 100us 定时中断基准。
 *          2. 设定 100 个时间片构成 10ms (100Hz) 的 PWM 周期, 避免人眼察觉闪烁。
 *          3. 在中断中动态调节占空比 (pwm_duty: 0 ~ 100), 每隔 20ms 微调一级亮度,
 *             实现由暗渐亮再由亮渐暗的呼吸灯光视觉效果。
 */

#include <REG52.H>

/* 引脚定义: P1.0 连接单颗 LED 阴极 */
sbit LED = P1^0;

/* PWM 与呼吸渐变控制变量 (使用 volatile 防止编译器过度优化) */
volatile unsigned char pwm_count = 0;    // PWM 时间片计数器 (0 ~ 99)
volatile unsigned char pwm_duty = 0;     // 当前高电平/点亮占空比 (0 ~ 100)
volatile unsigned char breath_count = 0; // 亮度调节节拍计数器
volatile unsigned char fade_up = 1;      // 渐变方向标志: 1-变亮, 0-变暗

/**
 * @brief 定时器0中断服务函数
 * @note  中断周期: 100us (基准时间片)
 */
void timer0_isr(void) interrupt 1
{
    pwm_count++;

    /* 周期达到 100 个时间片 (100 * 100us = 10ms) */
    if (pwm_count >= 100)
    {
        pwm_count = 0;

        /* 每隔 20ms (2个PWM周期) 调整一次占空比 */
        breath_count++;
        if (breath_count >= 2)
        {
            breath_count = 0;

            if (fade_up)
            {
                if (pwm_duty < 100)
                    pwm_duty++;  // 渐亮
                else
                    fade_up = 0; // 达到最大亮度，切换为渐暗
            }
            else
            {
                if (pwm_duty > 0)
                    pwm_duty--;  // 渐暗
                else
                    fade_up = 1; // 达到熄灭状态，切换为渐亮
            }
        }
    }

    /* 比较占空比生成 PWM 信号 (低电平点亮) */
    if (pwm_count < pwm_duty)
        LED = 0;  // 导通点亮
    else
        LED = 1;  // 截止熄灭
}

/**
 * @brief 主函数
 */
void main(void)
{
    LED = 1; // 初始状态关闭 LED

    /* 定时器0配置: 模式2 (8位自动重装载) */
    TMOD = (TMOD & 0xF0) | 0x02;

    /* 12MHz 晶振, 12T 模式下: 定时 100us 初值计算: 256 - (100 * 12 / 12) = 156 (0x9C) */
    TH0 = 0x9C;
    TL0 = 0x9C;

    ET0 = 1; // 允许定时器0中断
    EA = 1;  // 开启总中断
    TR0 = 1; // 启动定时器0

    while (1)
    {
        /* 主循环空闲等待中断驱动 PWM 呼吸 */
    }
}
