/**
 * @file main_15f.c
 * @brief STC15F 系列平台 (蓝桥杯 CT107D 架构) - 软件延时实现 LED 交替闪烁
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式单时钟周期单片机)
 *          硬件控制:
 *          1. 蓝桥杯开发板外设总线通过 74HC138 译码器 + 74HC573 锁存器架构进行控制。
 *          2. 将 J13 跳线帽接在 IO 模式，P4.2 输出低电平 (P42 = 0) 开启 WR 门禁。
 *          3. 配置 74HC138 译码通道 Y4C (LED 锁存器): P27=1, P26=0, P25=0。
 *          4. 74HC573 锁存器处于高电平有效 (LE=1) 透明导通状态。
 *          5. P0 端口输出低电平驱动板载 LED，L1 (P0.0) 与 L2 (P0.1) 轮流点亮，产生 500ms 节奏交替闪烁。
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
        P0 = 0xFE;       // 1111 1110B: 点亮 L1 (P0.0)，熄灭 L2~L8
        Delay500ms();    // 延时 500ms
        P0 = 0xFD;       // 1111 1101B: 点亮 L2 (P0.1)，熄灭 L1, L3~L8
        Delay500ms();    // 延时 500ms
    }
}
