/**
 * @file main.c
 * @brief STC15F2K60S2 蓝桥杯板载数码管基础动态扫描显示例程
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          硬件架构:
 *          1. 蓝桥杯数码管由 2 颗 74HC573 锁存器控制:
 *             - 位选锁存器 (Com/Digit): 由 74HC138 的 Y6C 选通 (P27=1, P26=1, P25=0)
 *             - 段选锁存器 (Segment):  由 74HC138 的 Y7C 选通 (P27=1, P26=1, P25=1)
 *          2. 门禁控制: J13 接 IO 模式，P4.2 输出低电平使能 WR 门禁。
 *          3. 消影机制: 在每次切换位选前，先向段选输出 0xFF (共阳极全灭)，
 *             消除视觉残影与鬼影现象。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/**
 * @brief 专为 STC15 (1T 模式) @11.0592MHz 适配的毫秒级软件阻塞延时
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

/**
 * @brief 显示单个数码管字符 (包含消影与动态刷新)
 * @param pos 位选掩码 (0x01: 第1位, 0x02: 第2位, ..., 0x80: 第8位)
 * @param dat 段选编码 (共阳极数码管编码: 低电平对应段导通点亮)
 */
void Display_One_Digit(unsigned char pos, unsigned char dat)
{
    /* 1. 消影处理: 选通 Y7C (段选锁存器)，输出 0xFF 熄灭数码管，防止切换位选时产生重影 */
    P27 = 1; P26 = 1; P25 = 1;
    P0 = 0xFF;                
    
    /* 2. 位选送数: 选通 Y6C (位选锁存器)，选择当前需要点亮的数码管物理位置 */
    P27 = 1; P26 = 1; P25 = 0;
    P0 = pos;                  
    
    /* 3. 段选送数: 再次选通 Y7C (段选锁存器)，输出要显示的字形段码 */
    P27 = 1; P26 = 1; P25 = 1;
    P0 = dat;                  

    /* 4. 短暂延时 1ms，利用人眼视觉暂留呈现稳定图像 */
    Delay(1); 
}

/**
 * @brief 主函数
 */
void main(void)
{
    /* 开启 74HC138 译码门禁 (J13 IO 模式) */
    P42 = 0;

    while (1)
    {
        /* 动态高频轮流扫描各数码管位置，利用视觉暂留实现多位数字稳定显示 */
        Display_One_Digit(0x01, 0xF9); // 第 1 位显示 '1' (共阳极段码 0xF9)
        Display_One_Digit(0x02, 0xA4); // 第 2 位显示 '2' (共阳极段码 0xA4)
        Display_One_Digit(0x04, 0xB0); // 第 3 位显示 '3' (共阳极段码 0xB0)
        Display_One_Digit(0x08, 0x99); // 第 4 位显示 '4' (共阳极段码 0x99)
        Display_One_Digit(0x10, 0x92); // 第 5 位显示 '5' (共阳极段码 0x92)
    }
}
