/**
 * @file DS1302.h
 * @brief DS1302 实时时钟芯片驱动头文件
 * @details 蓝桥杯板载 DS1302 串行 RTC 驱动接口
 */

#ifndef __DS1302_H__
#define __DS1302_H__

/**
 * @brief DS1302 引脚与接口初始化
 */
void DS1302_Init(void);

/**
 * @brief 向 DS1302 写入 1 字节命令与 1 字节数据
 * @param Command 命令控制字 (含地址与读写位)
 * @param Data 要写入的目标数据 (BCD 格式)
 */
void DS1302_WriteByte(unsigned char Command, unsigned char Data);

/**
 * @brief 从 DS1302 读取 1 字节数据
 * @param Command 读命令控制字 (含目标寄存器地址)
 * @return 读取到的 1 字节数据 (BCD 格式)
 */
unsigned char DS1302_ReadByte(unsigned char Command);

#endif
