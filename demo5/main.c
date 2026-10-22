/**
 * @file main.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 可调毫秒级延时实现流水灯 Plus
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式, 晶振 12.000MHz)
 *          引脚连接: P1 端口连接 8 颗 LED 阴极 (低电平点亮)
 *          功能说明: 封装参数化的毫秒级阻塞延时函数 Delay1ms(unsigned int xms)，
 *                   使延时时间具备高度灵活性，便于构建各种时长的动态灯光序列。
 */

#include <STC89C5xRC.H>

/**
 * @brief 可调毫秒阻塞延时函数
 * @param xms 需要延时的毫秒数 (范围 0 ~ 65535)
 * @note  时钟频率: 12.000MHz (12T 模式)
 */
void Delay1ms(unsigned int xms)	//@12.000MHz
{
	unsigned char data i, j;
	while (xms)
	{
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
		xms--;
	}
}

/**
 * @brief 主函数
 */
void main(void)
{
	while (1)
	{
		P1 = 0xFE;         // 点亮第 1 颗 LED
		Delay1ms(1000);    // 延时 1000ms (1秒)
		P1 = 0xFD;         // 点亮第 2 颗 LED
		Delay1ms(1000);    // 延时 1000ms
		P1 = 0xFF;         // 全部熄灭
		Delay1ms(500);     // 延时 500ms
		P1 = 0x00;         // 全部点亮
		Delay1ms(500);     // 延时 500ms
	}  
}
