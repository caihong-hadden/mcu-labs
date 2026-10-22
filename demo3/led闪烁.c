/**
 * @file led闪烁.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 软件延时实现 LED 交替闪烁
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式, 晶振 12.000MHz)
 *          引脚连接: P1 端口连接板载 LED (低电平点亮)
 *          功能说明: 利用三重嵌套循环实现约 500ms 的纯软件阻塞延时，
 *                   在死循环中让 P1.0 (LED1) 与 P1.1 (LED2) 轮流点亮并交替闪烁。
 */

#include <STC89C5xRC.H>
#include <INTRINS.H> 

/**
 * @brief 软件阻塞延时 500ms 函数
 * @note  时钟频率: 12.000MHz (传统 12T 模式单片机)
 */
void Delay500ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	i = 4;
	j = 205;
	k = 187;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/**
 * @brief 主函数
 */
void main(void)
{
	while (1)
	{
		P1 = 0xFE;       // 1111 1110B: 点亮 LED1 (P1.0)
		Delay500ms();    // 延时 500ms
		P1 = 0xFD;       // 1111 1101B: 点亮 LED2 (P1.1)
		Delay500ms();    // 延时 500ms
	}
}
