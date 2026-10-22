/**
 * @file main.c
 * @brief STC15F2K60S2 软件延时模拟 PWM 实现 LED 呼吸灯效果
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          核心原理:
 *          在双重循环中，通过动态改变高电平延时 Time 与低电平延时 (400 - Time) 的比例，
 *          平滑调节 LED 的点亮时间比例，实现无中断环境下的渐亮与渐暗呼吸灯效果。
 */

#include <STC15F2K60S2.H>

void Delay(unsigned int t)
{
	while (t--);
}

void main(void)
{
	unsigned int Time, i;

	/* 开启 Y4C (LED 锁存使能: P27=1, P26=0, P25=0) */
	P25 = 0;
	P26 = 0;
	P27 = 1;
	P42 = 1;

	while (1)
	{
		/* 1. 逐渐变亮阶段 (点亮时间 Time 从 0 递增到 400) */
		for (Time = 0; Time < 400; Time++)
		{
			for (i = 0; i < 10; i++)
			{
				P0 = 0x00;          // 点亮全部板载 LED (L1~L8)
				Delay(Time);        // 维持点亮时长
				P0 = 0xFF;          // 熄灭全部板载 LED
				Delay(400 - Time);  // 维持熄灭时长
			}
		}

		/* 2. 逐渐变暗阶段 (点亮时间 Time 从 400 递减到 0) */
		for (Time = 400; Time > 0; Time--)
		{
			for (i = 0; i < 10; i++)
			{
				P0 = 0x00;          // 点亮全部板载 LED
				Delay(Time);        // 维持点亮时长
				P0 = 0xFF;          // 熄灭全部板载 LED
				Delay(400 - Time);  // 维持熄灭时长
			}
		}
	}
}
