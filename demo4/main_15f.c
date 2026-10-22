/**
 * @file main_15f.c
 * @brief STC15F 系列平台 (蓝桥杯 CT107D 架构) - LED 状态流动与流水灯
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          硬件控制:
 *          1. 蓝桥杯板载外设采用 74HC138 译码器与 74HC573 锁存器管理。
 *          2. 跳线 J13 置于 IO 模式，P4.2 输出低电平 (P42 = 0) 开启 WR 门禁。
 *          3. 设置 74HC138: P27=1, P26=0, P25=0 选通 Y4C 通道使能 LED 锁存器。
 *          4. P0 端口按节拍输出 0xFE (L1亮)、0xFD (L2亮)、0xFF (全灭)、0x00 (全亮)，
 *             结合 STC15 1T 高精度 500ms 延时实现流水演示。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/**
 * @brief 专为 STC15 系列 (1T 模式) @11.0592MHz 适配的 500ms 延时函数
 */
void Delay500ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 29;
	j = 183;
	k = 25;
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
    /* 1. 开启 74HC138 译码与 LED (Y4C) 锁存使能 */
    P42 = 0;    // WR 门禁使能 (J13 IO 模式)
    P27 = 1;
    P26 = 0;
    P25 = 0;    // Y4C 选通使能 (74HC573 LE 保持高电平透明传输)

    while (1)
    {
        P0 = 0xFE;       // 1111 1110B: 点亮 L1 (P0.0)
        Delay500ms();
        P0 = 0xFD;       // 1111 1101B: 点亮 L2 (P0.1)
        Delay500ms();
        P0 = 0xFF;       // 1111 1111B: 全部熄灭
        Delay500ms();
        P0 = 0x00;       // 0000 0000B: 全部点亮 (L1~L8 全亮)
        Delay500ms();
    }
}
