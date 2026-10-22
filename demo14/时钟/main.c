/**
 * @file main.c
 * @brief STC15F2K60S2 定时器0倒计时时钟与蜂鸣器报警 (基础阻塞架构)
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能说明:
 *          1. LCD1602 显示 6 秒倒计时 (time: 6000ms);
 *          2. 定时器0每 1ms 递减倒计时间隔;
 *          3. 倒计时归零时，LCD1602 显示 "timeover!!!" 并选通 Y5C 驱动蜂鸣器鸣叫 2 秒。
 */

#include <STC15F2K60S2.H>
#include "timer0.h"
#include "lcd1602.h"

unsigned int time = 0; // 毫秒倒计时全局计数器

void main(void)
{
	LCD_Init();     // 初始化 LCD1602
	Timer0_Init();  // 初始化定时器0 (1ms中断)

	/* 初始上电短暂等待 200ms */
	time = 200;
	while (time > 0);

	while (1)
	{
		/* 1. 启动 6 秒倒计时并在 LCD 显示 */
		LCD_ShowString(1, 1, "time:       ");
		LCD_ShowString(1, 16, "s");
		time = 6000;
		while (time > 0)
		{
			LCD_ShowNum(1, 14, time / 1000, 2); // 实时刷新显示剩余秒数
		}

		/* 2. 倒计时结束，报警提示 2 秒 */
		time = 2000;
		LCD_ShowString(1, 1, "timeover!!!                 ");

		/* 选通 Y5C (P27=1, P26=0, P25=1)，打开蜂鸣器 (P0.6=1) */
		P27 = 1; P26 = 0; P25 = 1; P06 = 1;
		P27 = 0; P26 = 0; P25 = 0;

		while (time > 0); // 维持报警 2 秒

		/* 关闭蜂鸣器 */
		P27 = 1; P26 = 0; P25 = 1; P06 = 0;
		P27 = 0; P26 = 0; P25 = 0;
	}
}

/**
 * @brief 定时器0中断服务函数 (1ms 周期)
 */
void Timer0_Routine(void) interrupt 1
{
	if (time > 0)
	{
		time--; // 毫秒自减
	}
}
