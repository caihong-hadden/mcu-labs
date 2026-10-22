/**
 * @file main_15f.c
 * @brief STC15F 系列平台 (蓝桥杯 CT107D 架构) - 可调毫秒级延时实现流水灯 Plus
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          硬件控制:
 *          1. 蓝桥杯板载外设采用 74HC138 译码器与 74HC573 锁存器管理。
 *          2. 跳线 J13 置于 IO 模式，P4.2 输出低电平 (P42 = 0) 开启 WR 门禁。
 *          3. 设置 74HC138: P27=1, P26=0, P25=0 选通 Y4C 通道使能 LED 锁存器。
 *          4. 实现 1T 模式专用 Delay1ms(xms) 函数，通过传参实现灵活延时控制板载 L1~L8 流水动作。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/**
 * @brief 专为 STC15 系列 (1T 模式) @11.0592MHz 适配的可调毫秒延时函数
 * @param xms 延时时长 (ms)
 */
void Delay1ms(unsigned int xms)	//@11.0592MHz
{
	unsigned char data i, j;
	while (xms--)
	{
		_nop_();
		i = 11;
		j = 190;
		do
		{
			while (--j);
		} while (--i);
	}
}

/**
 * @brief 主函数
 */
void main(void)
{
    /* 1. 开启 74HC138 译码与 LED (Y4C) 锁存使能 */
    P42 = 0;    // WR 门禁使能 (J13 IO 模式)
    P27 = 1;
    P26 = 0;
    P25 = 0;    // Y4C 选通使能 (74HC573 LE 保持高电平透明传输)

    while (1)
    {
        P0 = 0xFE;         // 点亮 L1 (P0.0)
        Delay1ms(1000);    // 延时 1000ms (1秒)
        P0 = 0xFD;         // 点亮 L2 (P0.1)
        Delay1ms(1000);    // 延时 1000ms
        P0 = 0xFF;         // 全部熄灭
        Delay1ms(500);     // 延时 500ms
        P0 = 0x00;         // 全部点亮 (L1~L8 全亮)
        Delay1ms(500);     // 延时 500ms
    }  
}
