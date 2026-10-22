/**
 * @file main.c
 * @brief STC15F2K60S2 蓝桥杯板载数码管 8 位动态循环扫描显示
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          核心原理:
 *          1. 蓝桥杯数码管硬件架构:
 *             - Y6C 选通位选 (P27=1, P26=1, P25=0)
 *             - Y7C 选通段选 (P27=1, P26=1, P25=1)
 *          2. 通过共阳极字形编码表 NumberTable 将数字 0~9 映射到段码。
 *          3. 在主循环中轮流循环扫描 8 个物理数码管位置，分别显示 1 ~ 8。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/**
 * @brief 专为 STC15 (1T 模式) @11.0592MHz 适配的毫秒阻塞延时
 * @param xms 延时时长 (ms)
 */
void Delay(unsigned int xms)	//@11.0592MHz
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

/* 蓝桥杯共阳极数码管 0~9 字形段码表 */
unsigned char NumberTable[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

/**
 * @brief 数码管单物理位动态刷新函数
 * @param loc 数码管位置索引 (0 ~ 7, 对应第 1 ~ 8 位)
 * @param Num 显示的数值 (0 ~ 9)
 */
void Nixie(unsigned char loc, unsigned char Num)
{
    /* 1. 消影: 选通 Y7C 输出 0xFF 熄灭数码管，清除上一次的残影 */
    P27 = 1; P26 = 1; P25 = 1;
    P0 = 0xFF;

    /* 2. 送位选: 选通 Y6C，通过 1<<loc 选中指定位置的共阳公共端 */
    P27 = 1; P26 = 1; P25 = 0;
    P0 = (1 << loc);

    /* 3. 送段选: 选通 Y7C，查表输出对应数字的段码 */
    P27 = 1; P26 = 1; P25 = 1;
    P0 = NumberTable[Num];
}

/**
 * @brief 主函数
 */
void main(void)
{
    unsigned char j;

    /* 开启 74HC138 译码门禁 (J13 IO 模式) */
    P42 = 0;

    while (1)
    {
        /* 依次快速扫描 8 个数码管位置 (第 0~7 位分别显示数字 1~8) */
        for (j = 0; j < 8; j++)
        {
            Nixie(j, j + 1); // 刷新第 j 位显示数值 (j + 1)
            Delay(1);        // 维持 1ms 视觉暂留
        }
    }
}
