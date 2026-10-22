/**
 * @file Nixie.c
 * @brief 蓝桥杯板载共阳数码管底层驱动源文件
 * @details 通过 74HC138 译码器分时复用 P0 端口控制位选 (Y6C) 与段选 (Y7C)
 */

#include "Nixie.h"
#include <STC15F2K60S2.H>

/* 共阳极字形编码表: 0~9 数字段码, 0xFF 为消隐段码 */
unsigned char NumberTable[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF};

void Nixie(unsigned char loc, unsigned char Num)
{
    /* 1. 设置 74HC138 高位地址引脚 */
    P27 = 1; 
    P26 = 1;

    /* 2. 消影处理: 选通 Y7C (P25=1)，写入 0xFF 熄灭所有笔画，避免残影 */
    P25 = 1;
    P0 = 0xFF;

    /* 3. 送位选: 选通 Y6C (P25=0)，写入 1<<(loc-1) 选中第 loc 位数码管共阳端 */
    P25 = 0;
    P0 = (1 << (loc - 1));

    /* 4. 送段选: 选通 Y7C (P25=1)，查表输出对应数字的共阳极字形段码 */
    P25 = 1;
    P0 = NumberTable[Num];
}
