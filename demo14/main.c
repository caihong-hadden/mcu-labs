/**
 * @file main.c
 * @brief STC15F2K60S2 定时器0中断驱动 LED 流水灯与独立按键换向控制
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能说明:
 *          1. 定时器0每 1ms 中断一次，累计 500 次 (500ms) 触发一次 LED 移位;
 *          2. 支持按键换向: 每次按下独立按键 S7 (Key() 返回 1)，
 *             切换 LEDMODE (0-左循环移位, 1-右循环移位)。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>
#include "timer0.h"
#include "key.h"

unsigned char KeyNum = 0;   // 按键键码
unsigned char LEDMODE = 0;  // 移位模式: 0-左移, 1-右移

void main(void)
{
	/* 开启 Y4C (LED 锁存使能)，点亮 L1 作为初始状态 */
	P42 = 0;
	P27 = 1; P26 = 0; P25 = 0;
	P0 = 0xFE;

	Timer0_Init(); // 初始化定时器0 (1ms中断)

	while (1)
	{
		KeyNum = Key(); // 轮询独立按键
		if (KeyNum)
		{
			if (KeyNum == 1) // 按下 S7 按键
			{
				LEDMODE++;
				if (LEDMODE >= 2)
				{
					LEDMODE = 0; // 0 与 1 之间往复切换
				}
			}
		}
	}
}

/**
 * @brief 定时器0中断服务函数 (1ms 周期)
 */
void Timer0_Routine(void) interrupt 1
{
	static unsigned int T0Count = 0;
	T0Count++;

	/* 每 500ms 触发一次流水灯移位 */
	if (T0Count >= 500)
	{
		T0Count = 0;
		if (LEDMODE == 0)
		{
			P0 = _crol_(P0, 1); // 循环左移 1 位
		}
		else if (LEDMODE == 1)
		{
			P0 = _cror_(P0, 1); // 循环右移 1 位
		}
	}
}
