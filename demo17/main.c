/**
 * @file main.c
 * @brief STC15F2K60S2 DS1302 实时时钟读取并在 LCD1602 上显示秒数
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能说明:
 *          1. 初始化 DS1302 RTC 芯片并启动时钟计数 (解除写保护, 秒寄存器 CH 位写 0);
 *          2. 采用双采样防抖算法 Read_Safe_Second() 读取秒寄存器并转为十进制;
 *          3. 将实时秒数刷新至 LCD1602 液晶屏第 2 行。
 */

#include <STC15F2K60S2.H>
#include "LCD1602.h"
#include "DS1302.h"
#include "Delay.h"

/**
 * @brief 双读校验防抖读取秒数
 * @details 当读取瞬间恰好发生秒进位时可能产生偶发跳变，连续两次读数一致方为有效
 * @return 0~59 十进制秒数
 */
unsigned char Read_Safe_Second(void)
{
    unsigned char s1, s2;
    do
    {
        s1 = DS1302_ReadByte(0x81); // 0x81 为读秒寄存器命令
        s2 = DS1302_ReadByte(0x81);
    } while (s1 != s2); // 不一致则重读，避免读到进位过渡态
    
    return (s1 >> 4) * 10 + (s1 & 0x0F); // 将 BCD 码转换为十进制数
}

void main(void)
{
    unsigned char sec = 0;
    
    LCD_Init();     // 初始化 LCD1602
    Delay(20);
    DS1302_Init();  // 初始化 DS1302 接口

    LCD_ShowString(1, 1, "RTC Second:");
    
    /* 初始化时钟时间 (若晶振未起振，向 0x80 写 0 启动) */
    DS1302_WriteByte(0x8E, 0x00); // 关闭写保护
    DS1302_WriteByte(0x80, 0x00); // 设定秒为 0，且最高位 CH=0 启动内部振荡器
    DS1302_WriteByte(0x8E, 0x80); // 重新开启写保护
    
    while (1)
    {
        sec = Read_Safe_Second();      // 防抖读取当前十进制秒数
        LCD_ShowNum(2, 1, sec, 2);     // 在第 2 行显示 2 位秒数 (00~59)
        Delay(200);                    // 每 200ms 刷新一次界面
    }
}
