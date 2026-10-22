/**
 * @file uart.h
 * @brief 串口1 (UART1) 驱动模块头文件
 */

#ifndef __UART_H__
#define __UART_H__

/**
 * @brief 串口1初始化配置 (8位数据, 可变波特率, 启用中断与接收)
 */
void UART_Init(void);

/**
 * @brief 串口1发送单个字节数据
 * @param Byte 要发送的 8 位数据
 */
void UART_SendByte(unsigned char Byte);

#endif
