/**
 * @file main.c
 * @brief STC15F2K60S2 蓝桥杯板载无源/有源蜂鸣器驱动与救护车警报音效模拟
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 12.000MHz (STC 1T 模式)
 *          硬件控制:
 *          1. 蓝桥杯板载蜂鸣器与继电器经 ULN2003 达林顿管驱动，受 74HC573 锁存器 (Y5C) 控制。
 *          2. 选通 74HC138 译码器的 Y5 通道: P27=1, P26=0, P25=1, P42=1。
 *          3. P0.6 对应蜂鸣器控制端: 输出高电平 (1) 时鸣叫，输出低电平 (0) 时静音。
 *          4. 通过长短节拍切换，生动模拟真实救护车警报声响。
 */

#include <STC15F2K60S2.H>

/* 板载蜂鸣器位定义 (连接 ULN2003 输入端) */
sbit BUZZER = P0^6;

/**
 * @brief 专为 STC15 (1T 模式) @12.000MHz 适配的精确毫秒阻塞延时
 * @param ms 延时时长 (ms)
 */
void Delay_ms(unsigned int ms)
{
    unsigned char i, j;
    while (ms--)
    {
        i = 12; j = 169;
        do { while (--j); } while (--i);
    }
}

/**
 * @brief 救护车警报音效节奏模拟函数
 * @details 包含两段不同节奏：
 *          - 阶段 1: 急促连续短音 (5 次 50ms 鸣叫 + 50ms 间隔，共 500ms)
 *          - 阶段 2: 沉稳单次长音 (400ms 鸣叫 + 100ms 间隔，共 500ms)
 */
void Ambulance_Alarm(void)
{
    unsigned char i;
    
    /* 阶段 1: 急促连续短音 */
    for (i = 0; i < 5; i++)
    {
        BUZZER = 1;       // 蜂鸣器发声
        Delay_ms(50);
        BUZZER = 0;       // 蜂鸣器停止
        Delay_ms(50);
    }
    
    /* 阶段 2: 沉稳单次长音 */
    BUZZER = 1;           // 蜂鸣器长鸣
    Delay_ms(400);
    BUZZER = 0;           // 稍作停顿，区分轮次
    Delay_ms(100);
}

/**
 * @brief 主函数
 */
void main(void)
{
    /* 1. 打开 74HC138 译码器 Y5 通道 (选通蜂鸣器与继电器 74HC573 锁存器) */
    P27 = 1; P26 = 0; P25 = 1; P42 = 1; 
    
    /* 2. 上电默认关闭蜂鸣器，防止上电乱响 */
    BUZZER = 0; 

    while (1)
    {
        Ambulance_Alarm(); // 循环播放警报音效
    }
}
