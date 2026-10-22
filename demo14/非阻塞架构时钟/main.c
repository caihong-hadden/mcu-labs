/**
 * @file main.c
 * @brief STC15F2K60S2 非阻塞状态机架构倒计时时钟与报警系统
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          架构优势:
 *          彻底摒弃 while(time>0) 阻塞等待，采用前后台有限状态机 (FSM) 架构：
 *          - 定时器中断后台以 1ms 节拍递减各状态定时器 (time1, time2, time3);
 *          - 主循环前台实时根据状态 (state: 0/1/2) 流转处理界面与蜂鸣器动作，
 *            保证 CPU 永不卡死，为扩展多任务并发提供标准架构模板。
 */

#include <STC15F2K60S2.H>
#include "timer0.h"
#include "lcd1602.h"

/* 状态机各阶段时间片与状态变量 */
unsigned int time1 = 6000; // 倒计时阶段时长 (6000ms = 6s)
unsigned int time2 = 2000; // 报警阶段时长 (2000ms = 2s)
unsigned int time3 = 200;  // 上电启动缓冲时长 (200ms)
unsigned char state = 0;   // 状态机状态: 0-初始化等待, 1-倒计时, 2-报警响应

void main(void)
{
	/* 初始关闭蜂鸣器 */
	P27 = 1; P26 = 0; P25 = 1; P06 = 0;
	P27 = 0; P26 = 0; P25 = 0;

	Timer0_Init(); // 启动 1ms 定时器
	LCD_Init();    // 初始化 LCD1602 屏幕

	while (1)
	{
		switch (state)
		{
			case 0: // 状态 0: 启动缓冲结束，进入倒计时准备
				if (time3 == 0)
				{
					LCD_ShowString(1, 1, "time:       ");
					LCD_ShowString(1, 16, "s");
					state = 1; // 切换至状态 1
				}
				break;

			case 1: // 状态 1: 动态刷新倒计时剩余秒数
				LCD_ShowNum(1, 14, time1 / 1000, 2);
				if (time1 == 0)
				{
					LCD_ShowString(1, 1, "timeover!!!                 "); // 刷新提示语
					P27 = 1; P26 = 0; P25 = 1; P06 = 1;                     // 打开蜂鸣器
					P27 = 0; P26 = 0; P25 = 0;
					time2 = 2000;                                           // 设定报警 2 秒
					state = 2;                                              // 切换至状态 2
				}
				break;

			case 2: // 状态 2: 报警结束处理，循环复位
				if (time2 == 0)
				{
					P27 = 1; P26 = 0; P25 = 1; P06 = 0; // 关闭蜂鸣器
					P27 = 0; P26 = 0; P25 = 0;
					LCD_ShowString(1, 1, "time:       "); // 重置屏幕
					LCD_ShowString(1, 16, "s");
					time1 = 6000;                       // 重置 6 秒倒计时
					state = 1;                          // 循环回到状态 1
				}
				break;

			default:
				state = 0;
				break;
		}
	}
}

/**
 * @brief 定时器0中断服务函数 (1ms 周期)
 */
void Timer0_Routine(void) interrupt 1
{
	if (time1 > 0) time1--;
	if (time2 > 0) time2--;
	if (time3 > 0) time3--;
}
