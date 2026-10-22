/**
 * @file led流水灯.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 经典 LED 状态流动与流水灯
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式, 晶振 12.000MHz)
 *          引脚连接: P1 端口连接 8 颗 LED 阴极 (低电平点亮)
 *          功能说明: 依序设置 P1 端口的数据模式 (点亮单灯、全灭、全亮等状态)，
 *                   并通过 500ms 延时呈现直观的灯光流动与闪烁节奏。
 */

#include <STC89C5xRC.H>
#include <INTRINS.H>

/**
 * @brief 软件延时 500ms 函数
 * @note  时钟频率: 12.000MHz (12T 模式)
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
		P1 = 0xFE;       // 1111 1110B: 点亮第 1 颗 LED (P1.0)
		Delay500ms();
		P1 = 0xFD;       // 1111 1101B: 点亮第 2 颗 LED (P1.1)
		Delay500ms();
		P1 = 0xFF;       // 1111 1111B: 全部熄灭
		Delay500ms();
		P1 = 0x00;       // 0000 0000B: 全部点亮
		Delay500ms();
	}  
}
