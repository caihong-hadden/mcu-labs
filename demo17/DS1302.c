/**
 * @file DS1302.c
 * @brief DS1302 实时时钟芯片底层 3 线 SPI 通信驱动源文件
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          引脚连接:
 *          - SCLK (串行时钟): P1.7
 *          - IO   (双向数据): P2.3
 *          - CE   (片选使能): P1.3 (高电平有效)
 */

#include "DS1302.h"
#include <STC15F2K60S2.H>
#include <intrins.h>

sbit DS1302_SCLK = P1^7;
sbit DS1302_IO   = P2^3;
sbit DS1302_CE   = P1^3;

/**
 * @brief 专为 STC15 1T 高速模式适配的微秒级时钟建立延时 (约 3~5us)
 */
static void DS1302_Delay(void)
{
    unsigned char i = 10;
    while (--i);
}

void DS1302_Init(void)
{
    /* 强制将 P1.7, P1.3, P2.3 配置为标准准双向口模式 */
    P1M0 &= ~0x88; // 清除 bit7 与 bit3
    P1M1 &= ~0x88;
    P2M0 &= ~0x08; // 清除 bit3
    P2M1 &= ~0x08;

    DS1302_CE = 0;   // 初始片选拉低，通讯关闭
    DS1302_SCLK = 0; // 时钟初始低电平
    DS1302_IO = 1;   // 释放数据总线
}

void DS1302_WriteByte(unsigned char Command, unsigned char Data)
{
    unsigned char i;
    
    DS1302_CE = 1; // 拉高片选，开启通讯
    DS1302_Delay();
    
    /* 1. 写入命令字 (低位先行 LSB first) */
    for (i = 0; i < 8; i++)
    {
        DS1302_SCLK = 0;
        DS1302_IO = Command & 0x01;
        Command >>= 1;
        DS1302_Delay();
        DS1302_SCLK = 1; // 上升沿 DS1302 采样数据
        DS1302_Delay();
    }
    
    /* 2. 写入数据字节 (低位先行 LSB first) */
    for (i = 0; i < 8; i++)
    {
        DS1302_SCLK = 0;
        DS1302_IO = Data & 0x01;
        Data >>= 1;
        DS1302_Delay();
        DS1302_SCLK = 1; // 上升沿 DS1302 采样数据
        DS1302_Delay();
    }
    
    DS1302_SCLK = 0;
    DS1302_CE = 0; // 关闭片选
    DS1302_Delay();
}

unsigned char DS1302_ReadByte(unsigned char Command)
{
    unsigned char i;
    unsigned char Data = 0x00;
    
    DS1302_CE = 1; // 拉高片选
    DS1302_Delay();
    
    /* 1. 发送读命令字 (低位先行) */
    for (i = 0; i < 8; i++)
    {
        DS1302_SCLK = 0;
        DS1302_IO = Command & 0x01;
        Command >>= 1;
        DS1302_Delay();
        DS1302_SCLK = 1;
        DS1302_Delay();
    }
    
    /* 2. 释放 IO 总线准备接收数据 */
    DS1302_IO = 1; 
    DS1302_Delay();
    
    /* 3. 逐位读取数据 (DS1302 在时钟下降沿输出数据) */
    for (i = 0; i < 8; i++)
    {
        DS1302_SCLK = 1;
        DS1302_Delay();
        DS1302_SCLK = 0; // 下降沿
        DS1302_Delay();  // 稳定建立时间
        
        if (DS1302_IO)
        {
            Data |= (0x01 << i);
        }
        DS1302_Delay();
    }
    
    DS1302_CE = 0; // 关闭片选
    DS1302_Delay();
    return Data;
}
